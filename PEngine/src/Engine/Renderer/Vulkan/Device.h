#pragma once
#include "Core/PTypes.h"
#include "Core/PArray.h"
#include "Core/PArena.h"

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
	pstd::ArenaPair scratchArenas,
	VkInstance instance,
	VkSurfaceKHR surface
);
