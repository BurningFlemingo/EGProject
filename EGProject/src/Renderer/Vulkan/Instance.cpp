#include "Instance.h"
#include "PArray.h"

#include "ValidationLayers.h"
#include "Extensions.h"

#include "DebugMessenger.h"

VkInstance createInstance(pstd::FixedArena scratchArena) {
	pstd::FixedArray<const char*> foundExtensions{ findExtensions(&scratchArena
	) };

	pstd::FixedArray<const char*> foundValidationLayers{
		findValidationLayers(&scratchArena)
	};

	VkApplicationInfo appInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
							   .pApplicationName = "APPNAME",
							   .applicationVersion =
								   VK_MAKE_API_VERSION(0, 1, 0, 0),
							   .pEngineName = "NA",
							   .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
							   .apiVersion = VK_API_VERSION_1_0 };

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCI{
		getDebugMessengerCreateInfo()
	};

	VkInstanceCreateInfo vkInstanceCI{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = (uint32_t)pstd::getCapacity(foundValidationLayers),
		.ppEnabledLayerNames = pstd::getData(foundValidationLayers),
		.enabledExtensionCount = (uint32_t)pstd::getCapacity(foundExtensions),
		.ppEnabledExtensionNames = pstd::getData(foundExtensions),
	};
	if (debugMessengerCI.sType) {
		vkInstanceCI.pNext = &debugMessengerCI;
	}

	VkInstance instance{};
	VkResult res{ vkCreateInstance(&vkInstanceCI, nullptr, &instance) };
	ASSERT(res == VK_SUCCESS);

	return instance;
}
