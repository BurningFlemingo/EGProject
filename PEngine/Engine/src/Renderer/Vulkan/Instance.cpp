#include "Instance.h"
#include "ValidationLayers.h"
#include "Extensions.h"
#include "DebugMessenger.h"

#include "include/Logging.h"

#include "PArena.h"
#include "PArray.h"

#include "PContainer.h"
#include "PMemory.h"

#include "Platforms/VulkanSurface.h"
#include <vulkan/vulkan_core.h>

// TODO: finish this, then push it to dev
// after, refactor arrays to not use operator[] and instead indexRead /
// indexWrite. also get rid of constructor-like makeBoundedArray in favor of
// BoundedArray{}

VkInstance createInstance(pstd::ArenaFrame&& arenaFrame) {
	uint32_t extensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	pstd::Array<VkExtensionProperties> extensionProps{
		.allocation = pstd::scratchAlloc<VkExtensionProperties>(
			&arenaFrame, extensionCount
		),
	};
	vkEnumerateInstanceExtensionProperties(
		nullptr, &extensionCount, pstd::getData(extensionProps)
	);

	pstd::BoundedArray<const char*> requiredExtensions{
		.allocation = pstd::scratchAlloc<const char*>(&arenaFrame, 2),
		.count = 2
	};
	requiredExtensions[0] = Platform::getPlatformSurfaceExtension();
	requiredExtensions[1] = VK_KHR_SURFACE_EXTENSION_NAME;

	pstd::BoundedArray<const char*> optionalExtensions{
		pstd::makeBoundedArray<const char*>(getDebugExtensions().allocation)
	};
	pstd::Array<const char*> foundExtensions{ takeMatchedExtensions(
		pstd::makeFrame(arenaFrame, &arenaFrame.scratchOffset),
		extensionProps,
		&requiredExtensions,
		&optionalExtensions
	) };

	pstd::Array<const char*> foundValidationLayers{ findValidationLayers(
		pstd::makeFrame(arenaFrame, &arenaFrame.scratchOffset)
	) };

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

	pstd::Allocation rawAlloc{ pstd::alloc<const char*>(&arenaFrame, 1) };
	pstd::shallowMove(&rawAlloc, foundValidationLayers.allocation);

	VkInstanceCreateInfo vkInstanceCI{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = debugMessengerCI,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount =
			static_cast<uint32_t>(pstd::getCapacity(foundValidationLayers)),
		.ppEnabledLayerNames = pstd::getData(foundValidationLayers),
		.enabledExtensionCount =
			static_cast<uint32_t>(pstd::getCapacity(foundExtensions)),
		.ppEnabledExtensionNames = pstd::getData(foundExtensions),
	};

	VkInstance instance{};
	VkResult res{ vkCreateInstance(&vkInstanceCI, nullptr, &instance) };
	if (res != VK_SUCCESS) {
		LOG_ERROR("could not create vulkan instance: %i", (int)res);
	}

	return instance;
}
