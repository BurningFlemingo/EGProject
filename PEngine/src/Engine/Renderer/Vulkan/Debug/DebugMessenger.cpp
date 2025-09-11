#include "Engine/Renderer/Vulkan/DebugMessenger.h"

#include "Engine/Logging.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace {

	VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerUserCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	) {
		const char* severityString{};
		switch (messageSeverity) {
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				severityString = "[VULKAN VERBOSE]";
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				severityString = "[VULKAN INFO]";
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				severityString = "[VULKAN WARNING]";
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				severityString = "[VULKAN ERROR]";
				break;
			default:
				severityString = "[VULKAN ??]";
				break;
		}
		if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			Console::log(
				Console::LogLevel::none,
				"%m %m\n",
				severityString,
				pCallbackData->pMessage
			);
		}

		return false;
	}

	constexpr VkDebugUtilsMessengerCreateInfoEXT c_DebugUtilsMessengerCI{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debugMessengerUserCallback,
	};

	VkResult vkCreateDebugUtilsMessengerEXTThunk(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger
	) {
		auto functionPtr{ (PFN_vkCreateDebugUtilsMessengerEXT
		)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT") };

		if (!functionPtr) {
			ASSERT(false);
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
		return functionPtr(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	void vkDestroyDebugUtilsMessengerEXTThunk(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator
	) {
		auto functionPtr{ (PFN_vkDestroyDebugUtilsMessengerEXT
		)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT") };

		if (!functionPtr) {
			ASSERT(false);
			return;
		}
		functionPtr(instance, debugMessenger, pAllocator);
	}
}  // namespace

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance) {
	VkDebugUtilsMessengerEXT debugMessenger{};
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{
		*getDebugMessengerCreateInfo()
	};
	VkResult res{ vkCreateDebugUtilsMessengerEXTThunk(
		instance, &debugUtilsMessengerCI, nullptr, &debugMessenger
	) };
	if (res != VK_SUCCESS) {
		LOG_WARN("couldnt create debug messenger %i\n", (int)res);
	}

	return debugMessenger;
}

void destroyDebugMessenger(
	VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger
) {
	vkDestroyDebugUtilsMessengerEXTThunk(instance, debugMessenger, nullptr);
}

const VkDebugUtilsMessengerCreateInfoEXT* getDebugMessengerCreateInfo() {
	return &c_DebugUtilsMessengerCI;
}
