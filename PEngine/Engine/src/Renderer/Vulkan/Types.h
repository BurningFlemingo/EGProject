#pragma once
#include "Core/PArray.h"
#include "Core/PArray.h"
#include "Core/PFunction.h"

#include "Swapchain.h"
#include "Device.h"

#include <vulkan/vulkan.h>

namespace Renderer {
	struct State {
		Swapchain swapchain;
		Device device;
		VkSurfaceKHR surface;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPipeline graphicsPipeline;
		VkPipelineLayout graphicsPipelineLayout;
		uint32_t maxFramesInFlight;
		VkCommandPool cmdPool;
		pstd::Array<VkCommandBuffer> cmdBuffers;
		pstd::Array<VkSemaphore> imageAvailableSemaphores;
		pstd::Array<VkSemaphore> renderFinishedSemaphores;
		pstd::Array<VkFence> cmdBufferAvailableFences;

		uint32_t frameInFlight;
		pstd::DArray<pstd::Delegate<void()>*> deleters;
	};
}  // namespace Renderer
