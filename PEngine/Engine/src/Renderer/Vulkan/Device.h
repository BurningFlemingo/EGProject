#pragma once
#include "STD/PTypes.h"
#include "STD/PArray.h"
#include "STD/PArena.h"

#include <vulkan/vulkan.h>

enum class QueueFamily : uint32_t {
	graphics = 0,
	presentation = 1,
	transfer = 2,
	count
};

struct Device {
	VkPhysicalDevice physical;
	VkDevice logical;

	pstd::Array<uint32_t, QueueFamily> queueFamilyIndices;
	pstd::Array<VkQueue, QueueFamily> queues;
};

Device createDevice(
	pstd::Arena* pPersistArena,
	pstd::Arena scratchArena,
	VkInstance instance,
	VkSurfaceKHR surface
);
