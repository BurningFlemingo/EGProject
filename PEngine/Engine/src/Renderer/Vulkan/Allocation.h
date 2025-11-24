#pragma once
#include "STD/PTypes.h"
#include "Device.h"
#include <vulkan/vulkan.h>
#include "AssetLoader.h"

struct Buffer {
	VkBuffer handle;
	VkDeviceMemory memory;
	size_t size;
	size_t capacity;
	size_t alignment;
};

uint32_t getMemoryTypeIndex(
	uint32_t typeBits,
	VkMemoryPropertyFlags properties,
	VkPhysicalDeviceMemoryProperties memProps
);

Buffer createBuffer(
	const Device& device,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags memoryProps,
	size_t size
);

void copyBuffer(
	const Device& device,
	VkCommandPool pool,
	const Buffer& srcBuffer,
	const Buffer& dstBuffer,
	VkBufferCopy bufCopy
);
