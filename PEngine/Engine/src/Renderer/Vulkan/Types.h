#pragma once
#include <vulkan/vulkan.h>

namespace Renderer {
	struct State {
		VkSurfaceKHR surface;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
	};
}  // namespace Renderer
