#include "Renderer/Vulkan/DebugMessenger.h"

VkDebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo() {
	return {};
}

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance) {
	return {};
}
void destroyDebugMessenger(
	VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger
) {}
