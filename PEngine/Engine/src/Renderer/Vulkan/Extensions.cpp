#include "Extensions.h"

#include "include/Logging.h"

#include "PArena.h"
#include "PArray.h"
#include "PContainer.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace {
	pstd::Array<const char*> takeMatchedExtensions(
		pstd::ArenaFrame&& frame,
		pstd::BoundedArray<const char*>* extensionNamesToQuery,
		const pstd::Array<VkExtensionProperties>& availableExtensions
	);
}

pstd::Array<const char*> takeMatchedExtensions(
	pstd::ArenaFrame&& arenaFrame,
	const pstd::Array<VkExtensionProperties>& availableExtensionProps,
	pstd::BoundedArray<const char*>* pRequiredExtensions,
	pstd::BoundedArray<const char*>* pOptionalExtensions
) {
	pstd::Array<const char*> foundOptionalExtensions{};
	if (pOptionalExtensions != nullptr) {
		foundOptionalExtensions = takeMatchedExtensions(
			pstd::makeFrame(arenaFrame, &arenaFrame.scratchOffset),
			pOptionalExtensions,
			availableExtensionProps
		);
	}

	pstd::Array<const char*> foundRequiredExtensions{ takeMatchedExtensions(
		pstd::makeFrame(arenaFrame, &arenaFrame.scratchOffset),
		pRequiredExtensions,
		availableExtensionProps
	) };

	for (int i{}; i < pstd::getLength(*pRequiredExtensions); i++) {
		LOG_ERROR("could not find %m\n", (*pRequiredExtensions)[i]);
	}
	for (int i{}; i < pstd::getLength(*pOptionalExtensions); i++) {
		LOG_WARN("could not find %m\n", (*pOptionalExtensions)[i]);
	}
	for (int i{}; i < pstd::getLength(foundRequiredExtensions); i++) {
		LOG_INFO("found %m\n", foundRequiredExtensions[i]);
	}
	for (int i{}; i < pstd::getLength(foundOptionalExtensions); i++) {
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
	pstd::Array<const char*> takeMatchedExtensions(
		pstd::ArenaFrame&& frame,
		pstd::BoundedArray<const char*>* pExtensionNamesToQuery,
		const pstd::Array<VkExtensionProperties>& availableExtensions
	) {
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

		if (matchedNames.count == 0) {
			return {};
		}
		pstd::Array<const char*> res{
			.allocation = pstd::alloc<const char*>(&frame, matchedNames.count)
		};
		for (uint32_t i{}; i < matchedNames.count; i++) {
			res[i] = matchedNames[i];
			LOG_INFO("found %m", matchedNames[i]);
		}
		return res;
	}
}  // namespace
