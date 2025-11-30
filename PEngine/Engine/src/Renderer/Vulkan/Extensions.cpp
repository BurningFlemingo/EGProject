#include "Extensions.h"

#include "STD/PString.h"
#include "STD/PArray.h"
#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "STD/PContainer.h"
#include "Logging.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

pstd::Array<const char*> findExtensions(
	pstd::Arena* pPersistArena,
	const pstd::Span<pstd::String>& extensions,
	const pstd::Span<VkExtensionProperties>& availableExtensionProps
) {
	auto foundExtensions{
		pstd::createArray<const char*>(pPersistArena, extensions.count, 0)
	};

	for (size_t i{}; i < extensions.count; i++) {
		pstd::String queriedExtension{ extensions[i] };
		bool extensionFound{};
		for (size_t j{}; j < availableExtensionProps.count; j++) {
			const char* availableExtension{
				availableExtensionProps[j].extensionName
			};

			if (queriedExtension == availableExtension) {
				pstd::pushBack(&foundExtensions, availableExtension);
				extensionFound = true;
				break;
			}
		}
		if (extensionFound) {
			LOG_INFO("found %m\n", queriedExtension);
		} else {
			LOG_WARN("could not find %m\n", queriedExtension);
		}
	}

	return foundExtensions;
}
