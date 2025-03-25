#pragma once
#include "PTypes.h"
#include "PArray.h"
#include "PArena.h"

#include <vulkan/vulkan.h>

enum class QueueFamily : uint32_t { graphics = 0, presentation = 1, count };

struct Device {
	VkPhysicalDevice physical;
	VkDevice logical;

	pstd::Array<uint32_t, QueueFamily> queueFamilyIndices;
	pstd::Array<uint32_t> uniqueQueueFamilyIndices;

	pstd::Array<VkQueue, QueueFamily> queues;
};

Device createDevice(
	pstd::Arena* pPersistArena,
	pstd::LinkedArenaPair scratchArenas,
	VkInstance instance,
	VkSurfaceKHR surface
);
