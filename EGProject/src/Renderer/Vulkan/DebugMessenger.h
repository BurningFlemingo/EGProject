#pragma once

#include <vulkan/vulkan.h>

VkDebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo();

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance);

void destroyDebugMessenger(
	VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger
);
