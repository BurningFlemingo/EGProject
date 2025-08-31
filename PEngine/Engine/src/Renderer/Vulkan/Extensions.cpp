#include "Extensions.h"

#include "include/Logging.h"

#include "PArena.h"
#include "PArray.h"
#include "PMemory.h"
#include "PContainer.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace {
	pstd::Array<const char*> takeFoundExtensions(
		pstd::Arena* pPersistArena,
		pstd::ArenaPair scratchArenas,
		pstd::BoundedArray<const char*>* pExtensionNamesToQuery,
		const pstd::Array<VkExtensionProperties>& availableExtensions
	);
}

pstd::Array<const char*> takeFoundExtensions(
	pstd::Arena* pPersistArena,
	pstd::ArenaPair scratchArenas,
	const pstd::Array<VkExtensionProperties>& availableExtensionProps,
	pstd::BoundedArray<const char*>* pRequiredExtensions,
	pstd::BoundedArray<const char*>* pOptionalExtensions
) {
	ASSERT(pPersistArena);
	ASSERT(pRequiredExtensions);

	pstd::Array<const char*> foundOptionalExtensions{};
	if (pOptionalExtensions != nullptr) {
		foundOptionalExtensions = takeFoundExtensions(
			pPersistArena,
			scratchArenas,
			pOptionalExtensions,
			availableExtensionProps
		);
	}

	pstd::Array<const char*> foundRequiredExtensions{ takeFoundExtensions(
		pPersistArena,
		scratchArenas,
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
		pstd::Allocation allFoundExtensions{ pstd::makeConcatted<const char*>(
			pPersistArena,
			pstd::getAllocation(foundRequiredExtensions),
			pstd::getAllocation(foundOptionalExtensions)
		) };

		return { pstd::createArray<const char*>(allFoundExtensions) };
	}

	return foundRequiredExtensions;
}

namespace {
	pstd::Array<const char*> takeFoundExtensions(
		pstd::Arena* pPersistArena,
		pstd::ArenaPair scratchArenas,
		pstd::BoundedArray<const char*>* pExtensionNamesToQuery,
		const pstd::Array<VkExtensionProperties>& availableExtensions
	) {
		ASSERT(pPersistArena);
		pstd::Arena& scratchArena{
			*pstd::getUnique(&scratchArenas, pPersistArena)
		};

		size_t largestArrayViewCount{
			max(pExtensionNamesToQuery->count, availableExtensions.count)
		};
		size_t smallestArrayViewCount{
			min(pExtensionNamesToQuery->count, availableExtensions.count)
		};
		auto matchedNames{ pstd::createBoundedArray<const char*>(
			pPersistArena, largestArrayViewCount
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

		pstd::Array<const char*> res{ .data = matchedNames.data,
									  .count = matchedNames.count };
		return res;
	}
}  // namespace
