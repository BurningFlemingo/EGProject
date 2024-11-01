#pragma once
#include <vulkan/vulkan.h>

namespace Renderer::Internal {
	struct State {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
	};
}  // namespace Renderer::Internal
