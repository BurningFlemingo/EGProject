#include "Instance.h"
#include "ValidationLayers.h"
#include "Extensions.h"
#include "DebugMessenger.h"

#include "include/Logging.h"

#include "PArena.h"
#include "PArray.h"

#include "PContainer.h"
#include "PMemory.h"

VkInstance createInstance(pstd::FixedArena scratchArena) {
	pstd::FixedArray<const char*> foundExtensions{
		findExtensions(&scratchArena, scratchArena)
	};

	pstd::FixedArray<const char*> foundValidationLayers{
		findValidationLayers(&scratchArena, scratchArena)
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

	pstd::Allocation rawAlloc{
		pstd::arenaAlloc<const char*>(&scratchArena, 1)
	};
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
