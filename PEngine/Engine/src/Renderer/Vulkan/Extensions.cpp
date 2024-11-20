#include "Extensions.h"

#include "include/Logging.h"
#include "Platforms/VulkanSurface.h"

#include "PArena.h"
#include "PArray.h"
#include "PContainer.h"

#include <vulkan/vulkan.h>
#include <new>

pstd::Array<const char*> findExtensions(pstd::ArenaFrame&& arenaFrame) {
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

	pstd::BoundedArray<const char*> optionalExtensions{ getDebugExtensions() };

	pstd::BoundedArray<const char*> foundRequiredExtensions{
		.allocation = pstd::scratchAlloc<const char*>(&arenaFrame, 2)
	};
	pstd::BoundedArray<const char*> foundOptionalExtensions{
		.allocation = pstd::scratchAlloc<const char*>(
			&arenaFrame, optionalExtensions.count
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

	pstd::Array<const char*> res{
		.allocation = pstd::concat<const char*>(
			pstd::makeFrame(arenaFrame, arenaFrame.pPersistOffset),
			foundRequiredExtensions.allocation,
			foundOptionalExtensions.allocation
		)
	};
	return res;
}
