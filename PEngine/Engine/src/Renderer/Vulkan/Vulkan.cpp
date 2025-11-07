#include "STD/PVector.h"
#include "Renderer/Renderer.h"

#include "DebugMessenger.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "Allocation.h"
#include "Types.h"

#include "STD/PContainer.h"
#include "STD/PFileIO.h"
#include "STD/PArena.h"
#include "STD/PArray.h"
#include "STD/PString.h"
#include "Logging.h"
#include "Platforms/VulkanSurface.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <new>

struct Vertex {
	pstd::Vec4 pos;
	pstd::Vec4 color;
};

struct PushConstants {
	VkDeviceAddress vertexBufferAddress;
};

Renderer::State* Renderer::startup(
	pstd::Arena* pPersistArena,
	pstd::Arena scratchArena,
	const Platform::State& platformState
) {
	VkInstance instance{ createInstance(*pPersistArena, scratchArena) };

	VkDebugUtilsMessengerEXT debugMessenger{ createDebugMessenger(instance) };

	VkSurfaceKHR surface{ Platform::createSurface(instance, platformState) };

	Device device{
		createDevice(pPersistArena, scratchArena, instance, surface)
	};

	Swapchain swapchain{ createSwapchain(
		pPersistArena, scratchArena, device, surface, platformState
	) };

	pstd::String fragShaderString{ pstd::createString(
		pstd::readFile(&scratchArena, "shaders\\first.frag.spv")
	) };
	pstd::String vertShaderString{ pstd::createString(
		pstd::readFile(&scratchArena, "shaders\\first.vert.spv")
	) };

	VkShaderModuleCreateInfo fragmentShaderModuleCI{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = fragShaderString.size,
		.pCode = rcast<const uint32_t*>(fragShaderString.buffer)
	};

	VkShaderModuleCreateInfo vertexShaderModuleCI{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = vertShaderString.size,
		.pCode = rcast<const uint32_t*>(vertShaderString.buffer)
	};

	VkShaderModule fragShaderModule{};
	VkShaderModule vertShaderModule{};

	VkResult res{ vkCreateShaderModule(
		device.logical, &fragmentShaderModuleCI, nullptr, &fragShaderModule
	) };
	res = vkCreateShaderModule(
		device.logical, &vertexShaderModuleCI, nullptr, &vertShaderModule
	);

	VkPipelineShaderStageCreateInfo fragPipeCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShaderModule,
		.pName = "main",
	};
	VkPipelineShaderStageCreateInfo vertPipeCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertShaderModule,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertPipeCI, fragPipeCI };

	VkPipelineVertexInputStateCreateInfo vertInputCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	constexpr uint32_t nVertices{ 4 };
	Vertex vertices[nVertices]{
		Vertex{ .pos = pstd::Vec4{ -0.5, -0.5, 1.0, 1.0 },
				.color = pstd::Vec4{ 1.0, 0.0, 0.0, 1.0 } },
		Vertex{ .pos = pstd::Vec4{ 0.5, -0.5, 1.0, 1.0 },
				.color = pstd::Vec4{ 0.0, 1.0, 0.0, 1.0 } },
		Vertex{ .pos = pstd::Vec4{ 0.5, 0.5, 1.0, 1.0 },
				.color = pstd::Vec4{ 0.0, 0.0, 1.0, 1.0 } },
		Vertex{ .pos = pstd::Vec4{ -0.5, 0.5, 1.0, 1.0 },
				.color = pstd::Vec4{ 1.0, 1.0, 1.0, 1.0 } }
	};

	constexpr uint32_t nIndices{ 6 };
	uint16_t indices[nIndices]{ 0, 1, 2, 2, 3, 0 };

	VkCommandPoolCreateInfo transientCmdPoolCI{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		.queueFamilyIndex = device.queueFamilyIndices[QueueFamily::transfer],
	};

	VkCommandPool transientCmdPool{};
	vkCreateCommandPool(
		device.logical, &transientCmdPoolCI, nullptr, &transientCmdPool
	);

	Buffer stagingBuffer{ createBuffer(
		device,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		1024
	) };

	Buffer vBuffer{ createBuffer(
		device,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		sizeof(Vertex) * nVertices
	) };

	Buffer iBuffer{ createBuffer(
		device,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		sizeof(uint16_t) * nIndices
	) };

	VkBufferDeviceAddressInfo vAddressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = vBuffer.handle
	};

	VkDeviceAddress vertexDeviceAddress{
		vkGetBufferDeviceAddress(device.logical, &vAddressInfo)
	};

	void* mappedData{};
	vkMapMemory(
		device.logical,
		stagingBuffer.memory,
		0,
		stagingBuffer.size,
		0,
		&mappedData
	);
	memcpy(mappedData, vertices, sizeof(vertices));
	copyBuffer(device, transientCmdPool, stagingBuffer, vBuffer, vBuffer.size);

	memcpy(mappedData, indices, sizeof(indices));
	copyBuffer(device, transientCmdPool, stagingBuffer, iBuffer, iBuffer.size);

	VkDynamicState dynamicStates[]{ VK_DYNAMIC_STATE_SCISSOR,
									VK_DYNAMIC_STATE_VIEWPORT };

	VkPipelineDynamicStateCreateInfo dynamicCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates,
	};

	VkPipelineViewportStateCreateInfo viewportCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = false
	};

	VkPipelineRasterizationStateCreateInfo rasterizerCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.lineWidth = 1.f,
	};

	VkPipelineMultisampleStateCreateInfo multisampleCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.minSampleShading = 1.f
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState{
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo colorBlendCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachmentState,

	};

	VkPushConstantRange pushConstantRange{
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(PushConstants),
	};

	VkPipelineLayoutCreateInfo layoutCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange
	};

	VkPipelineLayout pipelineLayout{};
	vkCreatePipelineLayout(device.logical, &layoutCI, nullptr, &pipelineLayout);

	VkPipelineRenderingCreateInfo renderingCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &swapchain.createInfo.imageFormat
	};

	VkGraphicsPipelineCreateInfo pipelineCI{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &renderingCI,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertInputCI,
		.pInputAssemblyState = &inputAssemblyCI,
		.pViewportState = &viewportCI,
		.pRasterizationState = &rasterizerCI,
		.pMultisampleState = &multisampleCI,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &colorBlendCI,
		.pDynamicState = &dynamicCI,
		.layout = pipelineLayout,
	};

	VkPipeline graphicsPipeline{};
	res = vkCreateGraphicsPipelines(
		device.logical,
		VK_NULL_HANDLE,
		1,
		&pipelineCI,
		nullptr,
		&graphicsPipeline
	);

	vkDestroyShaderModule(device.logical, fragShaderModule, nullptr);
	vkDestroyShaderModule(device.logical, vertShaderModule, nullptr);

	VkCommandPoolCreateInfo cmdPoolCI{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = device.queueFamilyIndices[QueueFamily::graphics]
	};
	VkCommandPool cmdPool{};
	vkCreateCommandPool(device.logical, &cmdPoolCI, nullptr, &cmdPool);

	VkCommandBufferAllocateInfo cmdBufferAllocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = cmdPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkSemaphoreCreateInfo semaphoreCI{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	VkFenceCreateInfo fenceCI{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
							   .flags = VK_FENCE_CREATE_SIGNALED_BIT };

	const uint32_t maxFramesInFlight{ 2 };
	auto cmdBuffers{ pstd::createArray<VkCommandBuffer>(
		pPersistArena, swapchain.images.count
	) };
	auto cmdBufferAvailableFences{
		pstd::createArray<VkFence>(pPersistArena, maxFramesInFlight)
	};

	auto imageAvailableSemaphores{
		pstd::createArray<VkSemaphore>(pPersistArena, maxFramesInFlight)
	};

	auto renderFinishedSemaphores{
		pstd::createArray<VkSemaphore>(pPersistArena, swapchain.images.count)
	};

	for (uint32_t i{}; i < maxFramesInFlight; i++) {
		vkAllocateCommandBuffers(
			device.logical, &cmdBufferAllocInfo, &cmdBuffers[i]
		);
		vkCreateFence(
			device.logical, &fenceCI, nullptr, &cmdBufferAvailableFences[i]
		);
		vkCreateSemaphore(
			device.logical, &semaphoreCI, nullptr, &imageAvailableSemaphores[i]
		);
	}

	for (uint32_t i{}; i < swapchain.images.count; i++) {
		vkCreateSemaphore(
			device.logical, &semaphoreCI, nullptr, &renderFinishedSemaphores[i]
		);
	}

	State* state{ pstd::alloc<State>(pPersistArena) };
	return new (state) State{
		.swapchain = swapchain,
		.device = device,
		.surface = surface,
		.instance = instance,
		.debugMessenger = debugMessenger,
		.graphicsPipeline = graphicsPipeline,
		.graphicsPipelineLayout = pipelineLayout,
		.maxFramesInFlight = maxFramesInFlight,
		.cmdPool = cmdPool,
		.transientCmdPool = transientCmdPool,
		.cmdBuffers = cmdBuffers,
		.imageAvailableSemaphores = imageAvailableSemaphores,
		.renderFinishedSemaphores = renderFinishedSemaphores,
		.cmdBufferAvailableFences = cmdBufferAvailableFences,
		.stagingBufferData = mappedData,
		.stagingBuffer = stagingBuffer,
		.vertexBuffer = vBuffer,
		.vertexBufferDeviceAddress = vertexDeviceAddress,
		.indexBuffer = iBuffer,
	};
}

void Renderer::render(State* state, bool windowResized) {
	constexpr uint64_t uint64Max{ ~ncast<uint64_t>(0) };

	vkWaitForFences(
		state->device.logical,
		1,
		&state->cmdBufferAvailableFences[state->frameInFlight],
		VK_TRUE,
		uint64Max
	);

	uint32_t currentImageIndex{};
	VkResult res{ vkAcquireNextImageKHR(
		state->device.logical,
		state->swapchain.handle,
		uint64Max,
		state->imageAvailableSemaphores[state->frameInFlight],
		VK_NULL_HANDLE,
		&currentImageIndex
	) };
	ASSERT(res == VK_SUCCESS);

	VkCommandBufferBeginInfo cmdBufferBI{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	vkResetCommandBuffer(state->cmdBuffers[state->frameInFlight], 0);
	vkBeginCommandBuffer(state->cmdBuffers[state->frameInFlight], &cmdBufferBI);
	VkImageMemoryBarrier
		colorAttachmentFormatBarrier{ .sType =
										  VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
									  .dstAccessMask =
										  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
									  .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
									  .newLayout =
										  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
									  .srcQueueFamilyIndex =
										  state->device.queueFamilyIndices
											  [QueueFamily::graphics],
									  .dstQueueFamilyIndex =
										  state->device.queueFamilyIndices
											  [QueueFamily::graphics],
									  .image = state->swapchain
												   .images[currentImageIndex],
									  .subresourceRange = {
										  .aspectMask =
											  VK_IMAGE_ASPECT_COLOR_BIT,
										  .levelCount = 1,
										  .layerCount = 1,
									  } };

	vkCmdPipelineBarrier(
		state->cmdBuffers[state->frameInFlight],
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&colorAttachmentFormatBarrier
	);

	VkClearValue clearValue{ .color =
								 VkClearColorValue{ { 0.f, 0.f, 0.f, 1.f } } };

	VkRenderingAttachmentInfo colorAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = state->swapchain.imageViews[currentImageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = clearValue
	};
	VkRenderingInfo renderingInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea =
			VkRect2D{ .extent = state->swapchain.createInfo.imageExtent },
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentInfo
	};

	vkCmdBeginRendering(
		state->cmdBuffers[state->frameInFlight], &renderingInfo
	);
	vkCmdBindPipeline(
		state->cmdBuffers[state->frameInFlight],
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		state->graphicsPipeline
	);

	VkDeviceSize offsets[] = { 0 };

	vkCmdBindIndexBuffer(
		state->cmdBuffers[state->frameInFlight],
		state->indexBuffer.handle,
		offsets[0],
		VK_INDEX_TYPE_UINT16
	);

	VkViewport viewport{
		.width = ncast<float>(state->swapchain.createInfo.imageExtent.width),
		.height = ncast<float>(state->swapchain.createInfo.imageExtent.height),
		.minDepth = 0.f,
		.maxDepth = 1.f
	};
	VkRect2D scissor{ .extent = state->swapchain.createInfo.imageExtent };

	vkCmdSetViewport(state->cmdBuffers[state->frameInFlight], 0, 1, &viewport);
	vkCmdSetScissor(state->cmdBuffers[state->frameInFlight], 0, 1, &scissor);

	PushConstants pushConstants{ .vertexBufferAddress =
									 state->vertexBufferDeviceAddress };
	vkCmdPushConstants(
		state->cmdBuffers[state->frameInFlight],
		state->graphicsPipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(PushConstants),
		&pushConstants
	);

	vkCmdDrawIndexed(state->cmdBuffers[state->frameInFlight], 6, 1, 0, 0, 0);

	vkCmdEndRendering(state->cmdBuffers[state->frameInFlight]);

	VkImageMemoryBarrier
		presentFormatBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,

							  .srcAccessMask =
								  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
							  .oldLayout =
								  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							  .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
							  .srcQueueFamilyIndex =
								  state->device.queueFamilyIndices
									  [QueueFamily::graphics],
							  .dstQueueFamilyIndex =
								  state->device.queueFamilyIndices
									  [QueueFamily::presentation],
							  .image =
								  state->swapchain.images[currentImageIndex],
							  .subresourceRange = {
								  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
								  .levelCount = 1,
								  .layerCount = 1,
							  } };

	vkCmdPipelineBarrier(
		state->cmdBuffers[state->frameInFlight],
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&presentFormatBarrier
	);

	res = vkEndCommandBuffer(state->cmdBuffers[state->frameInFlight]);
	ASSERT(res == VK_SUCCESS);

	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};
	VkSubmitInfo renderSubmitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores =
			&state->imageAvailableSemaphores[state->frameInFlight],
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &state->cmdBuffers[state->frameInFlight],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores =
			&state->renderFinishedSemaphores[currentImageIndex],
	};

	vkResetFences(
		state->device.logical,
		1,
		&state->cmdBufferAvailableFences[state->frameInFlight]
	);
	vkQueueSubmit(
		state->device.queues[QueueFamily::graphics],
		1,
		&renderSubmitInfo,
		state->cmdBufferAvailableFences[state->frameInFlight]
	);

	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &state->renderFinishedSemaphores[currentImageIndex],
		.swapchainCount = 1,
		.pSwapchains = &state->swapchain.handle,
		.pImageIndices = &currentImageIndex,
	};

	vkQueuePresentKHR(
		state->device.queues[QueueFamily::presentation], &presentInfo
	);

	state->frameInFlight =
		(state->frameInFlight + 1) % state->maxFramesInFlight;
}

void Renderer::shutdown(State* state) {
	vkDeviceWaitIdle(state->device.logical);

	vkUnmapMemory(state->device.logical, state->stagingBuffer.memory);

	vkDestroyBuffer(state->device.logical, state->vertexBuffer.handle, nullptr);
	vkFreeMemory(state->device.logical, state->vertexBuffer.memory, nullptr);
	vkDestroyBuffer(state->device.logical, state->indexBuffer.handle, nullptr);
	vkFreeMemory(state->device.logical, state->indexBuffer.memory, nullptr);
	vkDestroyBuffer(
		state->device.logical, state->stagingBuffer.handle, nullptr
	);
	vkFreeMemory(state->device.logical, state->stagingBuffer.memory, nullptr);

	for (uint32_t i{}; i < state->maxFramesInFlight; i++) {
		vkDestroyFence(
			state->device.logical, state->cmdBufferAvailableFences[i], nullptr
		);
		vkDestroySemaphore(
			state->device.logical, state->imageAvailableSemaphores[i], nullptr
		);
	}

	for (uint32_t i{}; i < state->swapchain.images.count; i++) {
		vkDestroySemaphore(
			state->device.logical, state->renderFinishedSemaphores[i], nullptr
		);
	}

	vkDestroyCommandPool(state->device.logical, state->cmdPool, nullptr);
	vkDestroyCommandPool(
		state->device.logical, state->transientCmdPool, nullptr
	);
	vkDestroyPipeline(state->device.logical, state->graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(
		state->device.logical, state->graphicsPipelineLayout, nullptr
	);

	destroySwapchain(&state->swapchain, state->device.logical);
	vkDestroyDevice(state->device.logical, nullptr);
	vkDestroySurfaceKHR(state->instance, state->surface, nullptr);
	destroyDebugMessenger(state->instance, state->debugMessenger);
	vkDestroyInstance(state->instance, nullptr);
}
