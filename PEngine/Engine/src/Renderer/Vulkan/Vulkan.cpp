#include "Renderer/Renderer.h"

#include "DebugMessenger.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "Types.h"

#include "Core/PContainer.h"
#include "Core/PFileIO.h"
#include "Core/PArena.h"
#include "Core/PArray.h"
#include "Core/PString.h"
#include "Logging.h"
#include "Platforms/VulkanSurface.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <new>

Renderer::State* Renderer::startup(
	pstd::Arena* pPersistArena,
	pstd::Arena scratchArena,
	const Platform::State& platformState
) {
	VkInstance instance{ createInstance({ *pPersistArena, scratchArena }) };

	VkDebugUtilsMessengerEXT debugMessenger{ createDebugMessenger(instance) };

	VkSurfaceKHR surface{ Platform::createSurface(instance, platformState) };

	Device device{
		createDevice(pPersistArena, scratchArena, instance, surface)
	};

	Swapchain swapchain{ createSwapchain(
		pPersistArena, scratchArena, device, surface, platformState
	) };

	pstd::String fragShaderString{
		pstd::readFile(&scratchArena, "shaders\\first.frag.spv")
	};
	pstd::String vertShaderString{
		pstd::readFile(&scratchArena, "shaders\\first.vert.spv")
	};

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
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr,
	};

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
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
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
	};

	VkPipelineColorBlendStateCreateInfo colorBlendCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachmentState,
	};

	VkPipelineLayoutCreateInfo layoutCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	};

	VkPipelineLayout pipelineLayout{};
	vkCreatePipelineLayout(device.logical, &layoutCI, nullptr, &pipelineLayout);

	VkAttachmentDescription colorAttachmentDescription{
		.format = swapchain.createInfo.imageFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference colorAttachmentRef{
		.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpassDescription{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
	};

	VkRenderPassCreateInfo renderPassCI{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorAttachmentDescription,
		.subpassCount = 1,
		.pSubpasses = &subpassDescription,
	};

	VkRenderPass renderPass{};
	vkCreateRenderPass(device.logical, &renderPassCI, nullptr, &renderPass);

	VkGraphicsPipelineCreateInfo pipelineCI{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
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
		.renderPass = renderPass,
		.subpass = 0
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

	auto framebuffers{
		pstd::createArray<VkFramebuffer>(pPersistArena, swapchain.images.count)
	};
	for (uint32_t i{}; i < framebuffers.count; i++) {
		VkFramebufferCreateInfo framebufferCI{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = 1,
			.pAttachments = &swapchain.imageViews[i],
			.width = swapchain.createInfo.imageExtent.width,
			.height = swapchain.createInfo.imageExtent.height,
			.layers = 1,
		};

		vkCreateFramebuffer(
			device.logical, &framebufferCI, nullptr, &framebuffers[i]
		);
	}

	// device.queueFamilyIndices[QueueFamily::graphics];

	State* state{ pstd::alloc<State>(pPersistArena) };
	return new (state) State{ .swapchain = swapchain,
							  .device = device,
							  .surface = surface,
							  .instance = instance,
							  .debugMessenger = debugMessenger,
							  .renderPass = renderPass,
							  .graphicsPipeline = graphicsPipeline,
							  .graphicsPipelineLayout = pipelineLayout,
							  .framebuffers = framebuffers };
}

void Renderer::render(State* state) {}

void Renderer::shutdown(State* state) {
	for (uint32_t i{}; i < state->framebuffers.count; i++) {
		vkDestroyFramebuffer(
			state->device.logical, state->framebuffers[i], nullptr
		);
	}
	vkDestroyPipeline(state->device.logical, state->graphicsPipeline, nullptr);
	vkDestroyRenderPass(state->device.logical, state->renderPass, nullptr);
	vkDestroyPipelineLayout(
		state->device.logical, state->graphicsPipelineLayout, nullptr
	);

	destroySwapchain(&state->swapchain, state->device.logical);
	vkDestroyDevice(state->device.logical, nullptr);
	vkDestroySurfaceKHR(state->instance, state->surface, nullptr);
	destroyDebugMessenger(state->instance, state->debugMessenger);
	vkDestroyInstance(state->instance, nullptr);
}
