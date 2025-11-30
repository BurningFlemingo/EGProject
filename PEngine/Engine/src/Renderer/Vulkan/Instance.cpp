#include "Instance.h"
#include "ValidationLayers.h"
#include "Extensions.h"
#include "DebugMessenger.h"

#include "STD/PArena.h"
#include "STD/PArray.h"
#include "STD/PContainer.h"
#include "STD/PMemory.h"
#include "Logging.h"
#include "Platforms/VulkanSurface.h"
#include "STD/PAssert.h"

#include <vulkan/vulkan_core.h>

VkInstance createInstance(pstd::Arena scratchArena) {
	uint32_t extensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	auto extensionProps{
		pstd::createArray<VkExtensionProperties>(&scratchArena, extensionCount)
	};

	vkEnumerateInstanceExtensionProperties(
		nullptr, &extensionCount, extensionProps.data
	);

#ifdef DEBUG_BUILD
	pstd::String requiredExtensions[]{ Platform::getPlatformSurfaceExtension(),
									   VK_KHR_SURFACE_EXTENSION_NAME,
									   VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
#else
	pstd::String requiredExtensions[]{ Platform::getPlatformSurfaceExtension(),
									   VK_KHR_SURFACE_EXTENSION_NAME };
#endif

	pstd::Array<const char*> foundExtensions{
		findExtensions(&scratchArena, requiredExtensions, extensionProps)
	};

	pstd::Array<const char*> foundValidationLayers{
		findValidationLayers(&scratchArena)
	};

	VkApplicationInfo appInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
							   .pApplicationName = "APPNAME",
							   .applicationVersion =
								   VK_MAKE_API_VERSION(0, 1, 0, 0),
							   .pEngineName = "PEngine",
							   .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
							   .apiVersion = VK_API_VERSION_1_3 };

	const VkDebugUtilsMessengerCreateInfoEXT* debugMessengerCI{
		getDebugMessengerCreateInfo()
	};

	VkInstanceCreateInfo vkInstanceCI{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = debugMessengerCI,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(foundValidationLayers.count),
		.ppEnabledLayerNames = foundValidationLayers.data,
		.enabledExtensionCount = static_cast<uint32_t>(foundExtensions.count),
		.ppEnabledExtensionNames = foundExtensions.data,
	};

	VkInstance instance{};
	VkResult res{ vkCreateInstance(&vkInstanceCI, nullptr, &instance) };

	ASSERT(res == VK_SUCCESS, "could not create vulkan instance");

	return instance;
}
