#pragma once
#include "STD/PArray.h"
#include "STD/PArray.h"
#include "STD/PFunction.h"
#include "STD/PMatrix.h"

#include "Swapchain.h"
#include "Allocation.h"
#include "Device.h"
#include "EngineState.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct FrameCtx {
	VkCommandBuffer cmdBuffer;
	VkSemaphore imageAvailableSemaphore;
	VkFence renderFinishedFence;

	Buffer ubo;
	VkDescriptorSet uboSet;
};

struct Renderable {
	uint32_t indexOffset;
	uint32_t vertexOffset;

	uint32_t indexCount;

	Engine::Transform transform;
};

namespace Renderer {

	struct State {
		constexpr static uint32_t maxRenderables{ 1024 * 4 };
		constexpr static uint32_t maxFramesInFlight{ 2 };
		constexpr static size_t frameArenaSize{ 1024 * 1024 };
		pstd::StaticArray<pstd::Arena, maxFramesInFlight> frameArenas;

		Swapchain swapchain;
		Device device;
		VkSurfaceKHR surface;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPipeline graphicsPipeline;
		VkPipelineLayout graphicsPipelineLayout;

		VkCommandPool cmdPool;
		VkCommandPool transientCmdPool;

		VkDescriptorSetLayout uboSetLayout;
		VkDescriptorSetLayout bindlessSetLayout;

		VkDescriptorPool uboDescriptorPool;
		VkDescriptorPool bindlessDescriptorPool;

		VkDescriptorSet bindlessSet;

		pstd::Array<VkSemaphore> renderFinishedSemaphores;

		Buffer stagingBuffer;
		Buffer staticVertexBuffer;
		Buffer staticIndexBuffer;

		pstd::Array<FrameCtx> frameContexts;

		pstd::Array<Renderable> renderables;

		Image depthImage;
		Image textureImage;
		VkSampler textureSampler;

		pstd::Mat4 viewMatrix;
		pstd::Mat4 projectionMatrix;

		uint32_t frameInFlight;
	};
}  // namespace Renderer
