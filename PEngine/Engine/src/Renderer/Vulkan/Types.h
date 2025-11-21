#pragma once
#include "STD/PArray.h"
#include "STD/PArray.h"
#include "STD/PFunction.h"
#include "STD/PMatrix.h"

#include "Swapchain.h"
#include "Allocation.h"
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
		VkCommandPool transientCmdPool;

		pstd::Array<VkCommandBuffer> cmdBuffers;
		pstd::Array<VkSemaphore> imageAvailableSemaphores;
		pstd::Array<VkSemaphore> renderFinishedSemaphores;
		pstd::Array<VkFence> cmdBufferAvailableFences;

		void* stagingBufferData;

		Buffer stagingBuffer;
		Buffer vertexBuffer;
		VkDeviceAddress vertexBufferDeviceAddress;
		Buffer indexBuffer;
		pstd::Mat4 MVPMatrix;

		uint32_t frameInFlight;
		pstd::DArray<pstd::Delegate<void()>*> deleters;

		pstd::Arena frameArena;
	};
}  // namespace Renderer
