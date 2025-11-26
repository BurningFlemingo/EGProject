#include "Commands.h"
#include <vulkan/vulkan_core.h>

VkCommandBuffer beginTransientCmd(const Device& device, VkCommandPool pool) {
	VkCommandBufferAllocateInfo cmdBufferAllocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VkCommandBuffer cmdBuffer{};
	vkAllocateCommandBuffers(device.logical, &cmdBufferAllocInfo, &cmdBuffer);

	VkCommandBufferBeginInfo cmdBufBI{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	vkBeginCommandBuffer(cmdBuffer, &cmdBufBI);
	return cmdBuffer;
}
void endTransientCmd(const Device& device, VkCommandBuffer cmdBuffer) {
	vkEndCommandBuffer(cmdBuffer);

	VkCommandBufferSubmitInfo cmdSubmitInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = cmdBuffer
	};

	VkSubmitInfo2 submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdSubmitInfo,
	};

	VkResult res{ vkQueueSubmit2(
		device.queues[QueueFamily::transfer], 1, &submitInfo, nullptr
	) };
	ASSERT(res == VK_SUCCESS);

	vkQueueWaitIdle(device.queues[QueueFamily::transfer]);
}
