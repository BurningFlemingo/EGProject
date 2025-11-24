#pragma once
#include "STD/PArray.h"
#include "STD/PArray.h"
#include "STD/PFunction.h"
#include "STD/PMatrix.h"

#include "Swapchain.h"
#include "Allocation.h"
#include "Device.h"

#include <vulkan/vulkan.h>

struct FrameResources {
	Buffer vertexBuffer;
	Buffer indexBuffer;
	VkDeviceAddress vertexDeviceAddress;
};

namespace Renderer {
	struct State {
		constexpr static uint32_t maxFramesInFlight{ 2 };

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

		pstd::Array<FrameResources> frameResources;

		pstd::Mat4 MVPMatrix;

		pstd::Array<uint32_t> meshNIndices;
		pstd::Array<uint32_t> meshOffsets;

		uint32_t frameInFlight;
		pstd::DArray<pstd::Delegate<void()>*> deleters;

		pstd::Arena frameArena;
	};
}  // namespace Renderer
