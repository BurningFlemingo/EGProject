#pragma once
#include <vulkan/vulkan.h>

namespace Renderer {
	struct State {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
	};
}  // namespace Renderer
