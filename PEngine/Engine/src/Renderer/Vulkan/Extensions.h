#pragma once

#include "PArray.h"
#include <vulkan/vulkan.h>

pstd::Array<const char*> getDebugExtensions();

pstd::Array<const char*> takeMatchedExtensions(
	pstd::ArenaFrame&& arenaFrame,
	const pstd::Array<VkExtensionProperties>& availableExtensionProps,
	pstd::BoundedArray<const char*>* pRequiredExtensions,
	pstd::BoundedArray<const char*>* pOptionalExtensions = nullptr
);
