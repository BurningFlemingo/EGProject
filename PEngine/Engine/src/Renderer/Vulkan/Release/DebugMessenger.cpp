#include "Renderer/Vulkan/DebugMessenger.h"

const VkDebugUtilsMessengerCreateInfoEXT* getDebugMessengerCreateInfo() {
	return nullptr;
}

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance) {
	return {};
}
void destroyDebugMessenger(
	VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger
) {}
