#include "STD/PMemory.h"
#include "STD/PVector.h"
#include "Renderer.h"
#include "Renderer/Renderer.h"

#include "DebugMessenger.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "Allocation.h"
#include "Types.h"
#include "Pipeline.h"

#include "STD/PContainer.h"
#include "STD/PFileIO.h"
#include "STD/PArena.h"
#include "STD/PArray.h"
#include "STD/PString.h"
#include "STD/PMatrix.h"
#include "STD/PMath.h"
#include "Logging.h"
#include "EngineState.h"
#include "Platforms/VulkanSurface.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <new>

struct PushConstants {
	VkDeviceAddress vertexBufferAddress;
	alignas(16) pstd::Mat4 MVPMatrix;
};

struct UniformBufferObject {
	pstd::Mat4 modelMatrix;
	pstd::Mat4 viewMatrix;
	pstd::Mat4 projectionMatrix;
};

Renderer::State* Renderer::startup(
	pstd::AllocationRegistry* pAllocRegistry,
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

	pstd::StaticArray<VkPipelineShaderStageCreateInfo, 2> shaderStages{
		vertPipeCI, fragPipeCI
	};

	VkPushConstantRange pushConstantRange{
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(PushConstants),
	};
	pstd::StaticArray<VkFormat, 1> colorFormats{
		swapchain.createInfo.imageFormat
	};

	pstd::StaticArray<VkPushConstantRange, 1> pushConstantRanges{
		pushConstantRange
	};

	VkDescriptorSetLayoutBinding descriptorLayoutBinding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
	};

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &descriptorLayoutBinding,
	};

	VkDescriptorSetLayout descriptorSetLayout{};
	vkCreateDescriptorSetLayout(
		device.logical, &descriptorSetLayoutCI, nullptr, &descriptorSetLayout
	);

	VkPipelineLayout pipelineLayout{
		createPipelineLayout(device, pushConstantRanges, descriptorSetLayout)
	};

	VkPipeline graphicsPipeline{ createGraphicsPipeline(
		device, pipelineLayout, shaderStages, colorFormats
	) };

	vkDestroyShaderModule(device.logical, fragShaderModule, nullptr);
	vkDestroyShaderModule(device.logical, vertShaderModule, nullptr);

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
		1024 * 4
	) };

	auto frameContexts{
		pstd::createArray<FrameCtx>(pPersistArena, State::maxFramesInFlight)
	};

	for (size_t i{}; i < State::maxFramesInFlight; i++) {
		Buffer vBuffer{ createBuffer(
			device,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
				VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			1024 * 4
		) };

		Buffer iBuffer{ createBuffer(
			device,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			1024 * 4

		) };

		VkBufferDeviceAddressInfo vAddressInfo{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = vBuffer.handle,
		};

		VkDeviceAddress vertexDeviceAddress{
			vkGetBufferDeviceAddress(device.logical, &vAddressInfo)
		};

		FrameCtx frameCtx{
			.vertexBuffer = vBuffer,
			.indexBuffer = iBuffer,
			.vertexDeviceAddress = vertexDeviceAddress,

		};

		frameContexts[i] = frameCtx;
	}

	void* mappedData{};
	vkMapMemory(
		device.logical,
		stagingBuffer.memory,
		0,
		stagingBuffer.size,
		0,
		&mappedData
	);

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

	auto cmdBuffers{ pstd::createArray<VkCommandBuffer>(
		pPersistArena, swapchain.images.count
	) };
	auto cmdBufferAvailableFences{
		pstd::createArray<VkFence>(pPersistArena, State::maxFramesInFlight)
	};

	auto imageAvailableSemaphores{
		pstd::createArray<VkSemaphore>(pPersistArena, State::maxFramesInFlight)
	};

	auto renderFinishedSemaphores{
		pstd::createArray<VkSemaphore>(pPersistArena, swapchain.images.count)
	};

	for (uint32_t i{}; i < State::maxFramesInFlight; i++) {
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

	pstd::StaticArray<pstd::Arena, Renderer::State::maxFramesInFlight>
		frameArenas;

	for (size_t i{}; i < Renderer::State::maxFramesInFlight; i++) {
		frameArenas[i] = pstd::allocateArena(
			pAllocRegistry, Renderer::State::frameArenaSize
		);
	}

	auto renderables{ pstd::createArray<pstd::Array<Renderable>>(
		pPersistArena, Renderer::State::maxFramesInFlight
	) };

	State* state{ pstd::alloc<State>(pPersistArena) };
	return new (state)
		State{ .frameArenas = frameArenas,
			   .swapchain = swapchain,
			   .device = device,
			   .surface = surface,
			   .instance = instance,
			   .debugMessenger = debugMessenger,
			   .graphicsPipeline = graphicsPipeline,
			   .graphicsPipelineLayout = pipelineLayout,
			   .cmdPool = cmdPool,
			   .transientCmdPool = transientCmdPool,
			   .cmdBuffers = cmdBuffers,
			   .imageAvailableSemaphores = imageAvailableSemaphores,
			   .renderFinishedSemaphores = renderFinishedSemaphores,
			   .cmdBufferAvailableFences = cmdBufferAvailableFences,
			   .stagingBufferData = mappedData,
			   .stagingBuffer = stagingBuffer,
			   .frameContexts = frameContexts,
			   .renderables = renderables };
}

void Renderer::setModels(
	Renderer::State* pState,
	pstd::Arena scratchArena,
	pstd::Span<pstd::MeshData> meshes
) {
	if (meshes.count == 0) {
		return;
	}
	pstd::Arena* pFrameArena{ &pState->frameArenas[pState->frameInFlight] };

	auto renderables{
		pstd::createArray<Renderable>(pFrameArena, meshes.count)
	};

	size_t nVertices{};
	size_t nIndices{};
	for (size_t i{}; i < meshes.count; i++) {
		nVertices += meshes[i].uniquePositions.count;
		nIndices += meshes[i].indices.count;
	}
	auto vertices{ pstd::createArray<pstd::Vec4>(&scratchArena, nVertices, 0) };
	auto indices{ pstd::createArray<uint32_t>(&scratchArena, nIndices, 0) };

	for (size_t j{}; j < meshes.count; j++) {
		pstd::MeshData mesh{ meshes[j] };
		renderables[j] = {
			.indexOffset = ncast<uint32_t>(indices.count),
			.vertexOffset = ncast<uint32_t>(vertices.count),
			.indexCount = ncast<uint32_t>(mesh.indices.count),
		};

		for (size_t i{}; i < mesh.indices.count; i++) {
			uint32_t index{ mesh.indices[i] };
			pstd::pushBack(&indices, index);
		}

		for (size_t i{}; i < mesh.uniquePositions.count; i++) {
			pstd::pushBack(&vertices, mesh.uniquePositions[i]);
			pstd::Vec4 vertex{ vertices[vertices.count - 1] };
		}
	}

	size_t verticesByteSize{ vertices.count * sizeof(vertices[0]) };
	size_t indicesByteSize{ indices.count * sizeof(indices[0]) };

	vkDeviceWaitIdle(pState->device.logical);

	for (size_t i{}; i < Renderer::State::maxFramesInFlight; i++) {
		const FrameCtx& frameCtx{ pState->frameContexts[i] };

		memcpy(
			pState->stagingBufferData, vertices.data, verticesByteSize

		);

		copyBuffer(
			pState->device,
			pState->transientCmdPool,
			pState->stagingBuffer,
			frameCtx.vertexBuffer,
			{
				.size = verticesByteSize,
			}
		);

		memcpy(pState->stagingBufferData, indices.data, indicesByteSize);

		copyBuffer(
			pState->device,
			pState->transientCmdPool,
			pState->stagingBuffer,
			frameCtx.indexBuffer,
			{
				.size = indicesByteSize,
			}
		);

		pState->renderables[i] = renderables;
		pState->frameContexts[i] = frameCtx;
	}
}

void Renderer::setTransforms(
	Renderer::State* pState, pstd::Span<Engine::Transform> transforms
) {
	if (transforms.count == 0) {
		return;
	}
	pstd::Array<Renderable>* pRenderables{
		&pState->renderables[pState->frameInFlight]
	};
	for (size_t i{}; i < transforms.count; i++) {
		Renderable* renderable{ &(*pRenderables)[i] };
		renderable->transform = transforms[i];
	}
}

void Renderer::render(State* state, bool windowResized) {
	constexpr uint64_t uint64Max{ ~ncast<uint64_t>(0) };

	vkWaitForFences(
		state->device.logical,
		1,
		&state->cmdBufferAvailableFences[state->frameInFlight],
		VK_TRUE,
		UINT64_MAX
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

	VkViewport viewport{
		.y = ncast<float>(state->swapchain.createInfo.imageExtent.height),
		.width = ncast<float>(state->swapchain.createInfo.imageExtent.width),
		.height = -ncast<float>(state->swapchain.createInfo.imageExtent.height),
		.minDepth = 0.f,
		.maxDepth = 1.f
	};
	VkRect2D scissor{ .extent = state->swapchain.createInfo.imageExtent };

	vkCmdSetViewport(state->cmdBuffers[state->frameInFlight], 0, 1, &viewport);
	vkCmdSetScissor(state->cmdBuffers[state->frameInFlight], 0, 1, &scissor);

	const FrameCtx& frameCtx{ state->frameContexts[state->frameInFlight] };

	float ar{ 1920.0 / 1080.0 };
	pstd::Mat4 perspProjMatrix{
		pstd::calcPerspectiveMatrix(pstd::toRadians(90), ar, 0.001, 25)
	};

	pstd::Mat4 viewMatrix{
		pstd::calcLookAtMatrix({ 0.f, 0.f, 0.f }, { 0.f, 0.f, 1.f }, pstd::UP)
	};

	const pstd::Array<Renderable>& renderables{
		state->renderables[state->frameInFlight]
	};
	for (size_t i{}; i < renderables.count; i++) {
		const Renderable& renderable{ renderables[i] };

		pstd::Mat4 rotMat{ pstd::calcRotationMatrix<4>(renderable.transform.rot
		) };

		pstd::Mat4 modelMat{ pstd::calcTranlsated(
			pstd::getIdentityMatrix<4>(), renderable.transform.pos
		) };

		pstd::Mat4 mvpMatrix{ perspProjMatrix * viewMatrix * modelMat *
							  rotMat };

		PushConstants pushConstants{ .vertexBufferAddress =
										 frameCtx.vertexDeviceAddress,
									 .MVPMatrix = mvpMatrix };
		vkCmdPushConstants(
			state->cmdBuffers[state->frameInFlight],
			state->graphicsPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(PushConstants),
			&pushConstants
		);

		vkCmdBindIndexBuffer(
			state->cmdBuffers[state->frameInFlight],
			frameCtx.indexBuffer.handle,
			renderable.indexOffset * sizeof(uint32_t),
			VK_INDEX_TYPE_UINT32
		);

		vkCmdDrawIndexed(
			state->cmdBuffers[state->frameInFlight],
			renderable.indexCount,
			1,
			0,
			renderable.vertexOffset,
			0
		);
	}

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

	pstd::reset(&state->frameArenas[state->frameInFlight]);

	state->frameInFlight =
		(state->frameInFlight + 1) % state->maxFramesInFlight;
}

void Renderer::shutdown(State* state) {
	vkDeviceWaitIdle(state->device.logical);

	vkUnmapMemory(state->device.logical, state->stagingBuffer.memory);

	for (size_t i{}; i < State::maxFramesInFlight; i++) {
		const FrameCtx& frameCtx{ state->frameContexts[i] };

		vkDestroyBuffer(
			state->device.logical, frameCtx.vertexBuffer.handle, nullptr
		);
		vkFreeMemory(
			state->device.logical, frameCtx.vertexBuffer.memory, nullptr
		);

		vkDestroyBuffer(
			state->device.logical, frameCtx.indexBuffer.handle, nullptr
		);
		vkFreeMemory(
			state->device.logical, frameCtx.indexBuffer.memory, nullptr
		);
	}

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
