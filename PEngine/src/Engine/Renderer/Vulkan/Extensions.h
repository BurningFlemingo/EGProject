#pragma once

#include "Core/PArena.h"
#include "Core/PArray.h"

#include <vulkan/vulkan.h>

pstd::Array<const char*> getDebugExtensions();

pstd::Array<const char*> takeFoundExtensions(
	pstd::Arena* pPersistArena,
	pstd::Arena scratchArena,
	const pstd::Array<VkExtensionProperties>& availableExtensionProps,
	pstd::Array<const char*>* pRequiredExtensions,
	pstd::Array<const char*>* pOptionalExtensions = nullptr
);
