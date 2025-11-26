#pragma once
#include "STD/PTypes.h"
#include "Device.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "AssetLoader.h"

struct Buffer {
	VkBuffer handle;
	VkDeviceMemory memory;
	size_t size;
	size_t capacity;
	size_t alignment;
};

struct Image {
	VkImage handle;
	VkDeviceMemory memory;
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

Image create2DImage(
	const Device& device,
	VkMemoryPropertyFlags memoryProps,
	uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageUsageFlags usage,
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
	uint32_t mipLevels = 1
);

void copyBuffer(
	const Device& device,
	VkCommandPool pool,
	const Buffer& srcBuffer,
	const Buffer& dstBuffer,
	VkBufferCopy bufCopy
);
