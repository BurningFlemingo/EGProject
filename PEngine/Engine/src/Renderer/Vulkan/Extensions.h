#pragma once

#include "STD/PArena.h"
#include "STD/PArray.h"

#include <vulkan/vulkan.h>

pstd::Array<const char*> getDebugExtensions();

pstd::Array<const char*> takeFoundExtensions(
	pstd::Arena* pPersistArena,
	pstd::Arena scratchArena,
	const pstd::Array<VkExtensionProperties>& availableExtensionProps,
	pstd::Array<const char*>* pRequiredExtensions,
	pstd::Array<const char*>* pOptionalExtensions = nullptr
);
