#include "Instance.h"
#include "ValidationLayers.h"
#include "Extensions.h"
#include "DebugMessenger.h"

#include "Core/PArena.h"
#include "Core/PArray.h"
#include "Core/PContainer.h"
#include "Core/PMemory.h"
#include "Engine/Logging.h"
#include "Engine/Platforms/VulkanSurface.h"

#include <vulkan/vulkan_core.h>

VkInstance createInstance(pstd::ArenaPair scratchArenas) {
	pstd::Arena& scratchArena{ scratchArenas.primary };

	uint32_t extensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	auto extensionProps{
		pstd::createArray<VkExtensionProperties>(&scratchArena, extensionCount)
	};

	vkEnumerateInstanceExtensionProperties(
		nullptr, &extensionCount, extensionProps.data
	);

	auto requiredExtensions{
		pstd::createArray<const char*>(&scratchArena, 2, 0)
	};

	pstd::pushBack(
		&requiredExtensions, Platform::getPlatformSurfaceExtension()
	);

	pstd::pushBack(
		&requiredExtensions, ncast<const char*>(VK_KHR_SURFACE_EXTENSION_NAME)
	);

	auto optionalExtensions{ getDebugExtensions() };

	pstd::Array<const char*> foundExtensions{ takeFoundExtensions(
		&scratchArena,
		scratchArenas.secondary,
		extensionProps,
		&requiredExtensions,
		&optionalExtensions
	) };

	pstd::Array<const char*> foundValidationLayers{
		findValidationLayers(&scratchArena)
	};

	VkApplicationInfo appInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
							   .pApplicationName = "APPNAME",
							   .applicationVersion =
								   VK_MAKE_API_VERSION(0, 1, 0, 0),
							   .pEngineName = "NA",
							   .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
							   .apiVersion = VK_API_VERSION_1_0 };

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
	if (res != VK_SUCCESS) {
		LOG_ERROR("could not create vulkan instance: %i", (int)res);
	}

	return instance;
}
