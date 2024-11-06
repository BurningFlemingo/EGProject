#include "Extensions.h"

#include "include/Logging.h"
#include "Platforms/VulkanSurface.h"

#include "PArena.h"
#include "PArray.h"
#include "PContainer.h"

#include <vulkan/vulkan.h>
#include <new>

pstd::FixedArray<const char*> findExtensions(
	pstd::FixedArena* extensionsArena, pstd::FixedArena scratchArena
) {
	uint32_t extensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	pstd::FixedArray<VkExtensionProperties> extensionProps{
		.allocation = pstd::arenaAlloc<VkExtensionProperties>(
			extensionsArena, extensionCount
		),
	};
	vkEnumerateInstanceExtensionProperties(
		nullptr, &extensionCount, pstd::getData(extensionProps)
	);

	pstd::BoundedArray<const char*> requiredExtensions{
		.allocation = pstd::arenaAlloc<const char*>(&scratchArena, 2),
		.count = 2
	};
	new (pstd::getData(requiredExtensions))(const char* [2]
	){ Platform::getPlatformSurfaceExtension(), VK_KHR_SURFACE_EXTENSION_NAME };

	pstd::BoundedArray<const char*> optionalExtensions{ getDebugExtensions() };

	pstd::BoundedArray<const char*> foundRequiredExtensions{
		.allocation = pstd::arenaAlloc<const char*>(&scratchArena, 2)
	};
	pstd::BoundedArray<const char*> foundOptionalExtensions{
		.allocation = pstd::arenaAlloc<const char*>(
			&scratchArena, optionalExtensions.count
		),
	};

	for (int i{}; i < extensionCount; i++) {
		if (requiredExtensions.count == 0 && optionalExtensions.count == 0) {
			break;
		}
		char* avaliableExtension{ extensionProps[i].extensionName };
		size_t foundIndex{};
		auto matchFunction{ [&](const char* requiredExtension) {
			return pstd::stringsMatch(requiredExtension, avaliableExtension);
		} };

		if (pstd::find(requiredExtensions, matchFunction, &foundIndex)) {
			pstd::pushBack(
				&foundRequiredExtensions,
				requiredExtensions[foundIndex]	// ptr to string literal
			);
			pstd::compactRemove(&requiredExtensions, foundIndex);
		}
		if (pstd::find(optionalExtensions, matchFunction, &foundIndex)) {
			pstd::pushBack(
				&foundOptionalExtensions,
				optionalExtensions[foundIndex]	// ptr to string literal
			);
			pstd::compactRemove(&optionalExtensions, foundIndex);
		}
	}
	for (int i{}; i < requiredExtensions.count; i++) {
		LOG_ERROR("could not find %m\n", requiredExtensions[i]);
	}
	for (int i{}; i < optionalExtensions.count; i++) {
		LOG_WARN("could not find %m\n", optionalExtensions[i]);
	}
	for (int i{}; i < foundRequiredExtensions.count; i++) {
		LOG_INFO("found %m\n", foundRequiredExtensions[i]);
	}
	for (int i{}; i < foundOptionalExtensions.count; i++) {
		LOG_INFO("found %m\n", foundOptionalExtensions[i]);
	}

	pstd::FixedArray<const char*> res{ .allocation = pstd::concat<const char*>(
										   extensionsArena,
										   foundRequiredExtensions.allocation,
										   foundOptionalExtensions.allocation
									   ) };
	return res;
}
