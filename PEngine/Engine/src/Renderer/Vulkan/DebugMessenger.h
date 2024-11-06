#pragma once

#include <vulkan/vulkan.h>

const VkDebugUtilsMessengerCreateInfoEXT* getDebugMessengerCreateInfo();

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance);

void destroyDebugMessenger(
	VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger
);
