#pragma once
#include <vulkan/vulkan.h>

namespace Renderer {
	struct State {
		VkDevice device;
		VkPhysicalDevice physicalDevice;
		VkSurfaceKHR surface;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
	};
}  // namespace Renderer
