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

struct FrameCtx {
	Buffer vertexBuffer;
	Buffer indexBuffer;
	VkDeviceAddress vertexDeviceAddress;
};

struct Renderable {
	uint32_t indexOffset;
	uint32_t indexCount;

	Engine::Transform transform;
};

namespace Renderer {

	struct State {
		constexpr static uint32_t maxFramesInFlight{ 3 };
		constexpr static size_t frameArenaSize{ 1024 };
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

		pstd::Array<VkCommandBuffer> cmdBuffers;
		pstd::Array<VkSemaphore> imageAvailableSemaphores;
		pstd::Array<VkSemaphore> renderFinishedSemaphores;
		pstd::Array<VkFence> cmdBufferAvailableFences;

		void* stagingBufferData;
		Buffer stagingBuffer;

		pstd::Array<FrameCtx> frameContexts;

		pstd::Array<pstd::Array<Renderable>> renderables;

		uint32_t frameInFlight;
		pstd::DArray<pstd::Delegate<void()>*> deleters;

		pstd::Arena frameArena;
	};
}  // namespace Renderer
