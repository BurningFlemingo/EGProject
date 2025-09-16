#include "Extensions.h"

#include "Core/PString.h"
#include "Core/PArray.h"
#include "Core/PArena.h"
#include "Core/PMemory.h"
#include "Core/PContainer.h"
#include "Logging.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace {
	pstd::Array<const char*> takeFoundExtensions(
		pstd::Arena* pPersistArena,
		pstd::Arena scratchArena,
		pstd::Array<const char*>* pExtensionNamesToQuery,
		const pstd::Array<VkExtensionProperties>& availableExtensions
	);
}

pstd::Array<const char*> takeFoundExtensions(
	pstd::Arena* pPersistArena,
	pstd::Arena scratchArena,
	const pstd::Array<VkExtensionProperties>& availableExtensionProps,
	pstd::Array<const char*>* pRequiredExtensions,
	pstd::Array<const char*>* pOptionalExtensions
) {
	ASSERT(pPersistArena);
	ASSERT(pRequiredExtensions);

	pstd::Array<const char*> foundOptionalExtensions{};
	if (pOptionalExtensions != nullptr) {
		foundOptionalExtensions = takeFoundExtensions(
			pPersistArena,
			scratchArena,
			pOptionalExtensions,
			availableExtensionProps
		);
	}

	pstd::Array<const char*> foundRequiredExtensions{ takeFoundExtensions(
		pPersistArena,
		scratchArena,
		pRequiredExtensions,
		availableExtensionProps
	) };

	for (int i{}; i < pRequiredExtensions->count; i++) {
		LOG_ERROR("could not find %m\n", (*pRequiredExtensions)[i]);
	}
	if (pOptionalExtensions) {
		for (int i{}; i < pOptionalExtensions->count; i++) {
			LOG_WARN("could not find %m\n", (*pOptionalExtensions)[i]);
		}
	}

	for (int i{}; i < foundRequiredExtensions.count; i++) {
		LOG_INFO("found %m\n", foundRequiredExtensions[i]);
	}
	for (int i{}; i < foundOptionalExtensions.count; i++) {
		LOG_INFO("found %m\n", foundOptionalExtensions[i]);
	}

	if (foundOptionalExtensions.count > 0) {
		return pstd::makeConcatted<const char*>(
			pPersistArena, foundRequiredExtensions, foundOptionalExtensions
		);
	}

	return foundRequiredExtensions;
}

namespace {
	pstd::Array<const char*> takeFoundExtensions(
		pstd::Arena* pPersistArena,
		pstd::Arena scratchArena,
		pstd::Array<const char*>* pExtensionNamesToQuery,
		const pstd::Array<VkExtensionProperties>& availableExtensions
	) {
		ASSERT(pPersistArena);

		size_t largestArrayViewCount{
			max(pExtensionNamesToQuery->count, availableExtensions.count)
		};
		size_t smallestArrayViewCount{
			min(pExtensionNamesToQuery->count, availableExtensions.count)
		};
		auto matchedNames{ pstd::createArray<const char*>(
			pPersistArena, largestArrayViewCount, 0
		) };

		for (uint32_t i{}; i < largestArrayViewCount; i++) {
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

		return matchedNames;
	}
}  // namespace
