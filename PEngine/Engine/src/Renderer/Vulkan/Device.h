#pragma once
#include "PTypes.h"
#include "PArray.h"
#include "PArena.h"

#include <vulkan/vulkan.h>

enum class QueueFamily : uint32_t { graphics = 0, presentation = 1, count };

struct Device {
	VkPhysicalDevice physical;
	VkDevice logical;
	pstd::FixedArray<uint32_t, QueueFamily> queueFamilyIndices;
	pstd::FixedArray<VkQueue, QueueFamily> queues;
};

Device createDevice(
	pstd::FixedArenaFrame&& arenaFrame,
	VkInstance instance,
	VkSurfaceKHR surface
);
