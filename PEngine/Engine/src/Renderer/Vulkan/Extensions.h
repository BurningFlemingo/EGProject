#pragma once

#include "STD/PArena.h"
#include "STD/PArray.h"
#include "STD/PString.h"

#include <vulkan/vulkan.h>

pstd::Array<const char*> findExtensions(
	pstd::Arena* pPersistArena,
	const pstd::Span<pstd::String>& extensions,
	const pstd::Span<VkExtensionProperties>& availableExtensionProps
);
