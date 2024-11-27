#include "Extensions.h"

#include "include/Logging.h"
#include "Platforms/VulkanSurface.h"

#include "PArena.h"
#include "PArray.h"
#include "PContainer.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace {
	pstd::Array<const char*> stealMatchedExtensions(
		pstd::ArenaFrame&& frame,
		pstd::BoundedArray<const char*>* extensionNamesToQuery,
		const pstd::Array<VkExtensionProperties>& availableExtensions
	);
}

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

	pstd::BoundedArray<const char*> optionalExtensions{
		pstd::makeBoundedArray<const char*>(getDebugExtensions().allocation)
	};

	pstd::Array<const char*> foundRequiredExtensions{ stealMatchedExtensions(
		pstd::makeFrame(arenaFrame, &arenaFrame.scratchOffset),
		&requiredExtensions,
		extensionProps
	) };

	pstd::Array<const char*> foundOptionalExtensions{ stealMatchedExtensions(
		pstd::makeFrame(arenaFrame, &arenaFrame.scratchOffset),
		&optionalExtensions,
		extensionProps
	) };

	for (int i{}; i < pstd::getCapacity(requiredExtensions); i++) {
		LOG_ERROR("could not find %m\n", requiredExtensions[i]);
	}
	for (int i{}; i < pstd::getCapacity(optionalExtensions); i++) {
		LOG_WARN("could not find %m\n", optionalExtensions[i]);
	}
	for (int i{}; i < pstd::getCapacity(foundRequiredExtensions); i++) {
		LOG_INFO("found %m\n", foundRequiredExtensions[i]);
	}
	for (int i{}; i < pstd::getCapacity(foundOptionalExtensions); i++) {
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

namespace {
	pstd::Array<const char*> stealMatchedExtensions(
		pstd::ArenaFrame&& frame,
		pstd::BoundedArray<const char*>* pExtensionNamesToQuery,
		const pstd::Array<VkExtensionProperties>& availableExtensions
	) {
		ASSERT(pExtensionNamesToQuery);
		size_t largestArrayCount{
			max(pstd::getCapacity(*pExtensionNamesToQuery),
				pstd::getCapacity(availableExtensions))
		};
		size_t smallestArrayCount{
			min(pstd::getCapacity(*pExtensionNamesToQuery),
				pstd::getCapacity(availableExtensions))
		};
		pstd::BoundedArray<const char*> matchedNames{
			.allocation =
				pstd::scratchAlloc<const char*>(&frame, largestArrayCount)
		};

		for (uint32_t i{}; i < largestArrayCount; i++) {
			if (pExtensionNamesToQuery->count == 0) {
				break;
			}

			const char* avaliableExtension{
				availableExtensions[i].extensionName
			};

			auto matchFunction{ [&](const char* queriedExtension) {
				return pstd::stringsMatch(queriedExtension, avaliableExtension);
			} };

			size_t foundIndex{};
			if (pstd::find(
					*pExtensionNamesToQuery, matchFunction, &foundIndex
				)) {
				pstd::pushBack(
					&matchedNames, (*pExtensionNamesToQuery)[foundIndex]
				);
				pstd::compactRemove(pExtensionNamesToQuery, foundIndex);
			}
		}

		pstd::Array<const char*> res{
			.allocation = pstd::alloc<const char*>(&frame, matchedNames.count)
		};
		for (uint32_t i{}; i < matchedNames.count; i++) {
			res[i] = matchedNames[i];
		}
		return res;
	}
}  // namespace
