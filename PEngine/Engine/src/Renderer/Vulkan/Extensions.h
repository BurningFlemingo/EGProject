#pragma once

#include "PArena.h"
#include "PArray.h"
#include <vulkan/vulkan.h>

pstd::Array<const char*> getDebugExtensions();

pstd::Array<const char*> takeFoundExtensions(
	pstd::Arena* pPersistArena,
	pstd::ArenaPair scratchArenas,
	const pstd::Array<VkExtensionProperties>& availableExtensionProps,
	pstd::BoundedArray<const char*>* pRequiredExtensions,
	pstd::BoundedArray<const char*>* pOptionalExtensions = nullptr
);
