#pragma once
#include <vulkan/vulkan.h>
#include "PArray.h"
#include "Device.h"

namespace Renderer {
	struct State {
		VkSwapchainKHR swapchain;
		Device device;
		VkSurfaceKHR surface;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
	};
}  // namespace Renderer
