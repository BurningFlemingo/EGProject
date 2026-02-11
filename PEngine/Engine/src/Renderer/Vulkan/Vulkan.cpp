#include "AssetManager.h"
#include "STD/PCircularBuffer.h"
#include "STD/PHashMap.h"
#include "STD/PMemory.h"
#include "STD/PVector.h"
#include "STD/PSet.h"
#include "Camera.h"
#include "Renderer/Renderer.h"

#include "DebugMessenger.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "Allocation.h"
#include "Types.h"
#include "Pipeline.h"
#include "Commands.h"
#include "Descriptors.h"

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

#include "AssetLoader.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <new>

#include "STD/PSparseArray.h"

struct PushConstants {
	VkDeviceAddress vertexBufferAddress;
	alignas(16) pstd::Mat4 modelMatrix;
	alignas(16) uint32_t textureID;
};

struct UniformBufferObject {
	pstd::Mat4 viewMatrix;
	pstd::Mat4 projectionMatrix;
};

struct Vertex {
	pstd::Vec3 position;
	float u;
	pstd::Vec3 normal;
	float v;
};

Renderer::State* Renderer::startup(
	pstd::AllocationRegistry* pAllocRegistry,
	pstd::Arena* pPersistArena,
	pstd::Arena scratchArena,
	const Platform::State& platformState
) {
	VkInstance instance{ createInstance(scratchArena) };

	VkDebugUtilsMessengerEXT debugMessenger{ createDebugMessenger(instance) };

	VkSurfaceKHR surface{ Platform::createSurface(instance, platformState) };

	Device device{
		createDevice(pPersistArena, scratchArena, instance, surface)
	};

	Swapchain swapchain{ createSwapchain(
		pPersistArena, scratchArena, device, surface, platformState
	) };

	pstd::String fragShaderString{ pstd::createString(
		pstd::readFile(&scratchArena, "generated\\shaders\\first.frag.spv")
	) };
	pstd::String vertShaderString{ pstd::createString(
		pstd::readFile(&scratchArena, "generated\\shaders\\first.vert.spv")
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
		.data = { vertPipeCI, fragPipeCI }
	};

	VkPushConstantRange pushConstantRange{
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(PushConstants),
	};
	pstd::StaticArray<VkFormat, 1> colorFormats{
		.data = { swapchain.createInfo.imageFormat }
	};

	pstd::StaticArray<VkPushConstantRange, 1> pushConstantRanges{
		.data = { pushConstantRange }
	};

	VkPhysicalDeviceProperties physicalDeviceProps{};
	vkGetPhysicalDeviceProperties(device.physical, &physicalDeviceProps);

	uint32_t maxUniformBufferRange{
		physicalDeviceProps.limits.maxUniformBufferRange
	};

	uint32_t maxStorageBuffers{
		physicalDeviceProps.limits.maxDescriptorSetStorageBuffers
	};
	uint32_t maxSampledImages{
		physicalDeviceProps.limits.maxDescriptorSetSampledImages
	};
	uint32_t maxStorageImages{
		physicalDeviceProps.limits.maxDescriptorSetStorageImages
	};

	VkDescriptorSetLayoutBinding uniformBufferBinding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
	};

	VkDescriptorSetLayoutBinding bindlessDescriptorBinding{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = maxSampledImages,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};

	VkDescriptorBindingFlags bindlessBindingFlags{
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
		VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
	};

	VkDescriptorSetLayoutBindingFlagsCreateInfo bindlessSetLayoutSetFlags{
		.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.bindingCount = 1,
		.pBindingFlags = &bindlessBindingFlags
	};

	VkDescriptorSetLayoutCreateInfo bindlessSetLayoutCI{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &bindlessSetLayoutSetFlags,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = 1,
		.pBindings = &bindlessDescriptorBinding,
	};
	VkDescriptorSetLayoutCreateInfo uboSetLayoutCI{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &uniformBufferBinding,
	};

	VkDescriptorSetLayout bindlessSetLayout{};
	VkDescriptorSetLayout uboSetLayout{};
	vkCreateDescriptorSetLayout(
		device.logical, &bindlessSetLayoutCI, nullptr, &bindlessSetLayout
	);
	vkCreateDescriptorSetLayout(
		device.logical, &uboSetLayoutCI, nullptr, &uboSetLayout
	);

	VkPipelineLayout pipelineLayout{ createPipelineLayout(
		device,
		pushConstantRanges,
		pstd::StaticArray<VkDescriptorSetLayout, 2>{ uboSetLayout,
													 bindlessSetLayout }
	) };

	VkPipeline graphicsPipeline{ createGraphicsPipeline(
		device, pipelineLayout, shaderStages, colorFormats
	) };

	VkDescriptorPoolSize bindlessPoolSize{
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = maxSampledImages
	};

	auto uniformBufferSizes{ pstd::createArray<VkDescriptorPoolSize>(
		pPersistArena, Renderer::State::maxFramesInFlight, 0
	) };

	for (size_t i{}; i < Renderer::State::maxFramesInFlight; i++) {
		pstd::pushBack(
			&uniformBufferSizes,
			{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1 }
		);
	}

	VkDescriptorPoolCreateInfo bindlessPoolCI{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = 1,
		.poolSizeCount = 1,
		.pPoolSizes = &bindlessPoolSize
	};

	VkDescriptorPoolCreateInfo uniformBufferPoolCI{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = State::maxFramesInFlight,
		.poolSizeCount = ncast<uint32_t>(uniformBufferSizes.count),
		.pPoolSizes = uniformBufferSizes.data
	};

	VkDescriptorPool uboPool{};
	VkDescriptorPool bindlessPool{};
	vkCreateDescriptorPool(
		device.logical, &uniformBufferPoolCI, nullptr, &uboPool
	);

	vkCreateDescriptorPool(
		device.logical, &bindlessPoolCI, nullptr, &bindlessPool
	);

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
		1024 * 1024 * 256
	) };

	Engine::TextureData missingTexture{ Engine::loadTexture(
		pPersistArena, "generated\\textures\\Cobblestone.texture"
	) };

	size_t missingTextureSize{ missingTexture.width * missingTexture.height *
							   sizeof(missingTexture.pPixels[0]) };

	memcpy(
		stagingBuffer.pMappedData, missingTexture.pPixels, missingTextureSize
	);

	Image textureImage{ create2DImage(
		device,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		missingTexture.width,
		missingTexture.height,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
	) };

	Image depthImage{ create2DImage(
		device,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		swapchain.createInfo.imageExtent.width,
		swapchain.createInfo.imageExtent.height,
		VK_FORMAT_D32_SFLOAT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
	) };

	VkPhysicalDeviceProperties deviceProps;
	vkGetPhysicalDeviceProperties(device.physical, &deviceProps);
	VkSamplerCreateInfo samplerCI{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = deviceProps.limits.maxSamplerAnisotropy,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	};
	VkSampler sampler;
	vkCreateSampler(device.logical, &samplerCI, nullptr, &sampler);

	VkCommandBuffer cmdBuffer{ beginTransientCmd(device, transientCmdPool) };

	VkImageMemoryBarrier2
		transferBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
						 .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
						 .srcAccessMask = VK_ACCESS_2_NONE,
						 .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
						 .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT, 
						 .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
						 .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						 .image = textureImage.handle,
						 .subresourceRange = {
							 .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							 .baseMipLevel = 0,
							 .levelCount = 1,
							 .baseArrayLayer = 0,
							 .layerCount = 1,
						 },

		};
	VkDependencyInfo transferDependency{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &transferBarrier,
	};

	vkCmdPipelineBarrier2(cmdBuffer, &transferDependency);

	VkBufferImageCopy imageCopy {
		.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}, 
			.imageExtent = {
				.width = textureImage.width, 
				.height = textureImage.height, 
				.depth = 1
			}
	};

	vkCmdCopyBufferToImage(
		cmdBuffer,
		stagingBuffer.handle,
		textureImage.handle,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&imageCopy
	);

	VkImageMemoryBarrier2
		samplingBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
						 .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
						 .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
						 .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
						 .dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, 
						 .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						 .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						 .image = textureImage.handle,
						 .subresourceRange = {
							 .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							 .baseMipLevel = 0,
							 .levelCount = 1,
							 .baseArrayLayer = 0,
							 .layerCount = 1,
						 },

		};
	VkDependencyInfo samplingDependency{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &samplingBarrier,
	};
	vkCmdPipelineBarrier2(cmdBuffer, &samplingDependency);

	endTransientCmd(device, cmdBuffer);

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

	auto frameContexts{ pstd::createArray<FrameCtx>(
		pPersistArena, Renderer::State::maxFramesInFlight
	) };

	Buffer vBuffer{ createBuffer(
		device,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		Renderer::State::maxRenderables * sizeof(Vertex)
	) };

	Buffer iBuffer{ createBuffer(
		device,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		Renderer::State::maxRenderables * sizeof(uint32_t)

	) };

	for (size_t i{}; i < State::maxFramesInFlight; i++) {
		Buffer ubo{ createBuffer(
			device,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(UniformBufferObject)
		) };

		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = uboPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &uboSetLayout,
		};

		VkDescriptorSet uboSet{};
		vkAllocateDescriptorSets(
			device.logical, &descriptorSetAllocInfo, &uboSet
		);

		VkDescriptorBufferInfo bufferInfo{ .buffer = ubo.handle,
										   .range =
											   sizeof(UniformBufferObject) };

		VkWriteDescriptorSet uniformBufferWrite = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = uboSet,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &bufferInfo,
		};

		vkUpdateDescriptorSets(
			device.logical, 1, &uniformBufferWrite, 0, nullptr
		);

		VkCommandBuffer cmdBuffer{};
		VkFence renderFinishedFence{};
		VkSemaphore imageAvailableSemaphore{};

		vkAllocateCommandBuffers(
			device.logical, &cmdBufferAllocInfo, &cmdBuffer
		);
		vkCreateFence(device.logical, &fenceCI, nullptr, &renderFinishedFence);
		vkCreateSemaphore(
			device.logical, &semaphoreCI, nullptr, &imageAvailableSemaphore
		);

		FrameCtx frameCtx{
			.cmdBuffer = cmdBuffer,
			.imageAvailableSemaphore = imageAvailableSemaphore,
			.renderFinishedFence = renderFinishedFence,
			.ubo = ubo,
			.uboSet = uboSet,

		};

		frameContexts[i] = frameCtx;
	}

	VkDescriptorSetAllocateInfo bindlessDescriptorSetAllocInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = bindlessPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &bindlessSetLayout,
	};

	VkDescriptorSet bindlessSet{};
	vkAllocateDescriptorSets(
		device.logical, &bindlessDescriptorSetAllocInfo, &bindlessSet
	);

	VkDescriptorImageInfo imageInfo{
		.sampler = sampler,
		.imageView = textureImage.view,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet descriptorSetWrite = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = bindlessSet,
		.dstBinding = 0,
		.dstArrayElement = 1,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = &imageInfo
	};

	vkUpdateDescriptorSets(device.logical, 1, &descriptorSetWrite, 0, nullptr);

	vkDestroyShaderModule(device.logical, fragShaderModule, nullptr);
	vkDestroyShaderModule(device.logical, vertShaderModule, nullptr);

	auto renderFinishedSemaphores{
		pstd::createArray<VkSemaphore>(pPersistArena, swapchain.images.count)
	};

	for (uint32_t i{}; i < swapchain.images.count; i++) {
		vkCreateSemaphore(
			device.logical, &semaphoreCI, nullptr, &renderFinishedSemaphores[i]
		);
	}

	auto frameArenas{ pstd::createArray<pstd::Arena>(
		pPersistArena, Renderer::State::maxFramesInFlight
	) };

	for (size_t i{}; i < Renderer::State::maxFramesInFlight; i++) {
		pstd::Arena frameArena{
			pstd::allocateArena(pAllocRegistry, Renderer::State::frameArenaSize)
		};
		frameArenas[i] = frameArena;
	}

	auto renderables{ pstd::createArray<Renderable>(
		pPersistArena, Renderer::State::maxRenderables
	) };

	State* state{ pstd::alloc<State>(pPersistArena) };
	return new (state) State{
		.frameArenas = frameArenas,
		.staticArena =
			pstd::createArena(pPersistArena, Renderer::State::frameArenaSize),
		.swapchain = swapchain,
		.device = device,
		.surface = surface,
		.instance = instance,
		.debugMessenger = debugMessenger,
		.graphicsPipeline = graphicsPipeline,
		.graphicsPipelineLayout = pipelineLayout,
		.cmdPool = cmdPool,
		.transientCmdPool = transientCmdPool,
		.uboSetLayout = uboSetLayout,
		.bindlessSetLayout = bindlessSetLayout,
		.uboDescriptorPool = uboPool,
		.bindlessDescriptorPool = bindlessPool,
		.bindlessSet = bindlessSet,
		.renderFinishedSemaphores = renderFinishedSemaphores,
		.stagingBuffer = stagingBuffer,
		.staticVertexBuffer = vBuffer,
		.staticIndexBuffer = iBuffer,
		.assetIDToTextureID = pstd::createHashMap<AssetManager::UID, uint32_t>(
			pPersistArena, State::maxRenderables
		),
		.frameContexts = frameContexts,
		.renderables = renderables,
		.depthImage = depthImage,
		.textureImage = textureImage,
		.textureSampler = sampler,
	};
}

void Renderer::setCamera(State* pState, const Camera& camera) {
	float ar{ ncast<float>(pState->swapchain.createInfo.imageExtent.width) /
			  ncast<float>(pState->swapchain.createInfo.imageExtent.height) };

	pstd::Mat4 perspProjMatrix{ pstd::calcPerspectiveMatrix(
		camera.fovRadians, ar, camera.nearPlane, camera.farPlane
	) };

	pstd::Mat4 viewMatrix{
		pstd::calcLookAtMatrix(camera.transform.pos, camera.transform.rot)
	};

	pState->projectionMatrix = perspProjMatrix;
	pState->viewMatrix = viewMatrix;
}

void Renderer::setModels(
	Renderer::State* pState,
	AssetManager::State* pAssetManager,
	pstd::Arena scratchArena,
	pstd::Span<AssetManager::UID> meshIDs
) {
	pstd::reset(&pState->staticArena);

	if (meshIDs.count == 0) {
		return;
	}

	auto renderables{ pstd::createArray<Renderable>(
		&pState->staticArena, State::maxRenderables, 0
	) };

	auto uniqueUIDSet{
		pstd::createSet<AssetManager::UID>(&scratchArena, State::maxRenderables)
	};

	auto uniqueUIDs{ pstd::createArray<AssetManager::UID>(
		&scratchArena, State::maxRenderables, 0
	) };

	// TODO: make this 1. instanced and 2. into seperate drawcall struct
	uint32_t totalVertexCount{};
	uint32_t totalIndexCount{};

	uint32_t indexOffset{};
	uint32_t vertexOffset{};
	for (size_t i{}; i < meshIDs.count; i++) {
		AssetManager::UID uid{ meshIDs[i] };
		Engine::MeshData* pMesh{
			AssetManager::retrieveMesh(pAssetManager, uid)
		};

		if (!pstd::contains(uniqueUIDSet, uid)) {
			pstd::pushBack(&uniqueUIDs, uid);
			pstd::insert(&uniqueUIDSet, uid);

			indexOffset = totalIndexCount;

			totalVertexCount += pMesh->vertexCount;
			totalIndexCount += pMesh->indexCount;
		}

		// TODO: fix this
		Renderable renderable{ .indexOffset = indexOffset,
							   .vertexOffset = 0,
							   .indexCount = pMesh->indexCount };

		pstd::pushBack(&renderables, renderable);
	}

	auto vertices{
		pstd::createArray<Vertex>(&scratchArena, totalVertexCount, 0)
	};
	auto indices{
		pstd::createArray<uint32_t>(&scratchArena, totalIndexCount, 0)
	};

	for (size_t j{}; j < uniqueUIDs.count; j++) {
		Engine::MeshData* pMesh{
			AssetManager::retrieveMesh(pAssetManager, uniqueUIDs[j])
		};

		for (size_t i{}; i < pMesh->vertexCount; i++) {
			Vertex vertex{
				.position = pMesh->pPositions[i],
				.u = pMesh->pUVs[i].x,
				.normal = pMesh->pNormals[i],
				.v = pMesh->pUVs[i].y,
			};
			pstd::pushBack(&vertices, vertex);
		}

		for (size_t i{}; i < pMesh->indexCount; i++) {
			pstd::pushBack(&indices, pMesh->pIndices[i]);
		}
	}

	size_t verticesByteSize{ vertices.count * sizeof(Vertex) };
	size_t indicesByteSize{ indices.count * sizeof(uint32_t) };

	memcpy(
		pState->stagingBuffer.pMappedData, vertices.data, verticesByteSize

	);

	copyBuffer(
		pState->device,
		pState->transientCmdPool,
		pState->stagingBuffer,
		pState->staticVertexBuffer,
		{
			.size = verticesByteSize,
		}
	);

	memcpy(
		pState->stagingBuffer.pMappedData, indices.data, indicesByteSize

	);

	copyBuffer(
		pState->device,
		pState->transientCmdPool,
		pState->stagingBuffer,
		pState->staticIndexBuffer,
		{
			.size = indicesByteSize,
		}
	);

	pState->renderables = renderables;
}

void Renderer::setTransforms(
	Renderer::State* pState, pstd::Span<Engine::Transform> transforms
) {
	if (transforms.count == 0) {
		return;
	}
	for (size_t i{}; i < transforms.count; i++) {
		Renderable* renderable{ &pState->renderables[i] };
		renderable->transform = transforms[i];
	}
}

void Renderer::render(
	State* state, const Platform::State& platformState, bool windowResized
) {
	const FrameCtx& frame{ state->frameContexts[state->frameInFlight] };
	pstd::Arena framelocalArena{ state->frameArenas[state->frameInFlight] };
	if (windowResized) {
		vkDeviceWaitIdle(state->device.logical);
		refreshSwapchain(
			&state->swapchain, state->device, state->surface, platformState
		);
		return;
	}

	constexpr uint64_t uint64Max{ ~ncast<uint64_t>(0) };

	vkWaitForFences(
		state->device.logical,
		1,
		&frame.renderFinishedFence,
		VK_TRUE,
		UINT64_MAX
	);

	uint32_t currentImageIndex{};
	VkResult res{ vkAcquireNextImageKHR(
		state->device.logical,
		state->swapchain.handle,
		uint64Max,
		frame.imageAvailableSemaphore,
		VK_NULL_HANDLE,
		&currentImageIndex
	) };
	ASSERT(res == VK_SUCCESS);

	VkCommandBufferBeginInfo cmdBufferBI{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	vkResetCommandBuffer(frame.cmdBuffer, 0);
	vkBeginCommandBuffer(frame.cmdBuffer, &cmdBufferBI);

	constexpr uint32_t nMemoryBarriers{ 2 };
	VkImageMemoryBarrier2
		preFormatBarriers[nMemoryBarriers]{ {.sType =
										  VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
									  .srcStageMask =
										  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
									  .dstStageMask =
										  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
									  .dstAccessMask =
										  VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
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
									  }},  {.sType =
										  VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
									  .srcStageMask =
										  VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
									  .dstStageMask =
										  VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
									  .dstAccessMask =
										  VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT_KHR,
									  .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
									  .newLayout =
										  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,

									  .srcQueueFamilyIndex =
										  state->device.queueFamilyIndices
											  [QueueFamily::graphics],
									  .dstQueueFamilyIndex =
										  state->device.queueFamilyIndices
											  [QueueFamily::graphics],
									  .image = state->depthImage.handle,
									  .subresourceRange = {
										  .aspectMask =
											  VK_IMAGE_ASPECT_DEPTH_BIT,
										  .levelCount = 1,
										  .layerCount = 1,
									  }} };

	VkDependencyInfo preRenderDependency{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 2,
		.pImageMemoryBarriers = preFormatBarriers,
	};

	vkCmdPipelineBarrier2(frame.cmdBuffer, &preRenderDependency);

	constexpr VkClearValue colorClearValue{ .color = VkClearColorValue{
												{ 0.f, 0.f, 0.f, 0.f } } };
	VkClearValue depthClearValue{ .depthStencil = VkClearDepthStencilValue{
									  .depth = 1.f, .stencil = 1 } };

	VkRenderingAttachmentInfo colorAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = state->swapchain.imageViews[currentImageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = colorClearValue
	};
	VkRenderingAttachmentInfo depthAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = state->depthImage.view,
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = depthClearValue
	};

	VkRenderingInfo renderingInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea =
			VkRect2D{ .extent = state->swapchain.createInfo.imageExtent },
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentInfo,
		.pDepthAttachment = &depthAttachmentInfo,
	};

	vkCmdBeginRendering(frame.cmdBuffer, &renderingInfo);
	vkCmdBindPipeline(
		frame.cmdBuffer,
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

	vkCmdSetViewport(frame.cmdBuffer, 0, 1, &viewport);
	vkCmdSetScissor(frame.cmdBuffer, 0, 1, &scissor);

	const FrameCtx& frameCtx{ state->frameContexts[state->frameInFlight] };

	vkCmdBindIndexBuffer(
		frame.cmdBuffer,
		state->staticIndexBuffer.handle,
		0,
		VK_INDEX_TYPE_UINT32
	);

	UniformBufferObject ubo{ .viewMatrix = state->viewMatrix,
							 .projectionMatrix = state->projectionMatrix };

	memcpy(frame.ubo.pMappedData, &ubo, sizeof(UniformBufferObject));

	vkCmdBindDescriptorSets(
		frame.cmdBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		state->graphicsPipelineLayout,
		0,
		1,
		&frame.uboSet,
		0,
		nullptr
	);
	vkCmdBindDescriptorSets(
		frame.cmdBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		state->graphicsPipelineLayout,
		1,
		1,
		&state->bindlessSet,
		0,
		nullptr
	);

	for (size_t i{}; i < state->renderables.count; i++) {
		const Renderable& renderable{ state->renderables[i] };

		pstd::Mat4 rotMat{ pstd::calcRotationMatrix<4>(renderable.transform.rot
		) };

		pstd::Mat4 scaleMat{
			pstd::calcDiagonalMatrix<4>(renderable.transform.scale)
		};

		pstd::Mat4 translationMat{ pstd::calcTranslated(
			pstd::getIdentityMatrix<4>(), renderable.transform.pos
		) };

		pstd::Mat4 modelMat{ translationMat * scaleMat * rotMat };

		PushConstants pushConstants{
			.vertexBufferAddress = state->staticVertexBuffer.deviceAddress,
			.modelMatrix = modelMat,
			.textureID = 1
		};
		vkCmdPushConstants(
			frame.cmdBuffer,
			state->graphicsPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(PushConstants),
			&pushConstants
		);

		vkCmdDrawIndexed(
			frame.cmdBuffer,
			renderable.indexCount,
			1,
			renderable.indexOffset,
			renderable.vertexOffset,
			0
		);
	}

	vkCmdEndRendering(frame.cmdBuffer);

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
		frame.cmdBuffer,
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

	res = vkEndCommandBuffer(frame.cmdBuffer);
	ASSERT(res == VK_SUCCESS);

	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};
	VkSubmitInfo renderSubmitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &frame.imageAvailableSemaphore,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &frame.cmdBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores =
			&state->renderFinishedSemaphores[currentImageIndex],
	};

	vkResetFences(state->device.logical, 1, &frame.renderFinishedFence);
	vkQueueSubmit(
		state->device.queues[QueueFamily::graphics],
		1,
		&renderSubmitInfo,
		frame.renderFinishedFence
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

	for (size_t i{}; i < State::maxFramesInFlight; i++) {
		const FrameCtx& frame{ state->frameContexts[i] };

		vkDestroyFence(
			state->device.logical, frame.renderFinishedFence, nullptr
		);
		vkDestroySemaphore(
			state->device.logical, frame.imageAvailableSemaphore, nullptr
		);
		destroyBuffer(state->device, frame.ubo);
	}

	destroyBuffer(state->device, state->stagingBuffer);
	destroyBuffer(state->device, state->staticVertexBuffer);
	destroyBuffer(state->device, state->staticIndexBuffer);

	destroyImage(state->device, state->depthImage);
	destroyImage(state->device, state->textureImage);
	vkDestroySampler(state->device.logical, state->textureSampler, nullptr);

	vkDestroyDescriptorPool(
		state->device.logical, state->uboDescriptorPool, nullptr
	);
	vkDestroyDescriptorPool(
		state->device.logical, state->bindlessDescriptorPool, nullptr
	);

	vkDestroyDescriptorSetLayout(
		state->device.logical, state->bindlessSetLayout, nullptr
	);
	vkDestroyDescriptorSetLayout(
		state->device.logical, state->uboSetLayout, nullptr
	);

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
