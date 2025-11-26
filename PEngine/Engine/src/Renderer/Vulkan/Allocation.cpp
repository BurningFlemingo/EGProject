#include "Allocation.h"
#include "Logging.h"
#include "Commands.h"

#include <vulkan/vulkan_core.h>

uint32_t getMemoryTypeIndex(
	uint32_t typeBits,
	VkMemoryPropertyFlags properties,
	VkPhysicalDeviceMemoryProperties memProps
) {
	for (int i{}; i < memProps.memoryTypeCount; i++) {
		VkMemoryType memType{ memProps.memoryTypes[i] };
		bool matchedType{ (typeBits & (1 << i)) != 0 };
		bool matchedProps{ (memType.propertyFlags & properties) == properties };

		if (matchedType && matchedProps) {
			return i;
		}
	}

	LOG_ERROR("Couldn't find valid memory type...");
	return 0;
}

Buffer createBuffer(
	const Device& device,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags memoryProps,
	size_t size
) {
	VkBufferCreateInfo bufferCI{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VkBuffer buffer{};
	vkCreateBuffer(device.logical, &bufferCI, nullptr, &buffer);

	VkMemoryRequirements memReqs{};
	vkGetBufferMemoryRequirements(device.logical, buffer, &memReqs);

	VkPhysicalDeviceMemoryProperties physicalMemProps{};
	vkGetPhysicalDeviceMemoryProperties(device.physical, &physicalMemProps);

	uint32_t memTypeIndex{ getMemoryTypeIndex(
		memReqs.memoryTypeBits, memoryProps, physicalMemProps
	) };

	VkMemoryAllocateFlagsInfo allocFlagsInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
	};
	VkMemoryAllocateInfo memAllocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &allocFlagsInfo,
		.allocationSize = memReqs.size,
		.memoryTypeIndex = memTypeIndex,
	};
	if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
		allocFlagsInfo.flags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	}

	VkDeviceMemory memory{};
	vkAllocateMemory(device.logical, &memAllocInfo, nullptr, &memory);
	vkBindBufferMemory(device.logical, buffer, memory, 0);

	return Buffer{
		.handle = buffer,
		.memory = memory,
		.size = size,
		.capacity = memReqs.size,
		.alignment = memReqs.alignment,
	};
}

Image create2DImage(
	const Device& device,
	VkMemoryPropertyFlags memoryProps,
	uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageUsageFlags usage,
	VkSampleCountFlagBits samples,
	uint32_t mipLevels
) {
	VkImageCreateInfo imageCI{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = { .width = width, .height = height, .depth = 1 },
		.mipLevels = mipLevels,
		.arrayLayers = 1,
		.samples = samples,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VkImage image{};
	VkResult res{ vkCreateImage(device.logical, &imageCI, nullptr, &image) };
	ASSERT(res == VK_SUCCESS, "could not create image resource");

	VkMemoryRequirements memReqs{};
	vkGetImageMemoryRequirements(device.logical, image, &memReqs);

	VkPhysicalDeviceMemoryProperties physicalMemProps{};
	vkGetPhysicalDeviceMemoryProperties(device.physical, &physicalMemProps);

	uint32_t memTypeIndex{ getMemoryTypeIndex(
		memReqs.memoryTypeBits, memoryProps, physicalMemProps
	) };

	VkMemoryAllocateInfo memAllocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReqs.size,
		.memoryTypeIndex = memTypeIndex,
	};

	VkDeviceMemory memory{};
	vkAllocateMemory(device.logical, &memAllocInfo, nullptr, &memory);
	vkBindImageMemory(device.logical, image, memory, 0);

	return Image{
		.handle = image, .memory = memory, .width = width, .height = height
	};
}

void copyBuffer(
	const Device& device,
	VkCommandPool pool,
	const Buffer& srcBuffer,
	const Buffer& dstBuffer,
	VkBufferCopy bufCopy
) {
	ASSERT((bufCopy.size + bufCopy.srcOffset) <= srcBuffer.size);
	ASSERT((bufCopy.size + bufCopy.dstOffset) <= dstBuffer.size);

	VkCommandBuffer cmdBuffer{ beginTransientCmd(device, pool) };
	vkCmdCopyBuffer(cmdBuffer, srcBuffer.handle, dstBuffer.handle, 1, &bufCopy);
	endTransientCmd(device, cmdBuffer);
}
