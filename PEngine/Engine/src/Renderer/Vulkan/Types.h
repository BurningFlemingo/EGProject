#pragma once
#include "STD/PArray.h"
#include "STD/PHashMap.h"
#include "STD/PFunction.h"
#include "STD/PMatrix.h"

#include "Swapchain.h"
#include "Allocation.h"
#include "Device.h"
#include "EngineState.h"

#include "AssetManager.h"

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
	uint32_t textureID;

	Engine::Transform transform;
};

namespace Renderer {

	struct State {
		constexpr static uint32_t maxRenderables{ 1024 * 4 };
		constexpr static uint32_t maxFramesInFlight{ 2 };
		constexpr static size_t frameArenaSize{ 1024 * 1024 };
		pstd::Array<pstd::Arena> frameArenas;
		pstd::Arena staticArena;

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

		pstd::HashMap<AssetManager::UID, uint32_t> assetIDToTextureID;

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
