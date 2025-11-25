#include "Pipeline.h"

#include <vulkan/vulkan_core.h>

VkPipeline createGraphicsPipeline(
	const Device& device,
	const VkPipelineLayout layout,
	const pstd::Span<VkPipelineShaderStageCreateInfo> shaderStages,
	const pstd::Span<VkFormat> colorFormats
) {
	VkPipelineVertexInputStateCreateInfo vertInputCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
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

	VkPipelineRenderingCreateInfo renderingCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = ncast<uint32_t>(colorFormats.count),
		.pColorAttachmentFormats = colorFormats.data,
	};

	VkGraphicsPipelineCreateInfo pipelineCI{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &renderingCI,
		.stageCount = ncast<uint32_t>(shaderStages.count),
		.pStages = shaderStages.data,
		.pVertexInputState = &vertInputCI,
		.pInputAssemblyState = &inputAssemblyCI,
		.pViewportState = &viewportCI,
		.pRasterizationState = &rasterizerCI,
		.pMultisampleState = &multisampleCI,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &colorBlendCI,
		.pDynamicState = &dynamicCI,
		.layout = layout,
	};

	VkPipeline pipeline{};
	VkResult res{ vkCreateGraphicsPipelines(
		device.logical, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline
	) };

	ASSERT(res == VK_SUCCESS);

	return pipeline;
}

VkPipelineLayout createPipelineLayout(
	const Device& device,
	const pstd::Span<VkPushConstantRange> pushConstantRanges,
	VkDescriptorSetLayout descriptorSetLayout
) {
	VkPipelineLayoutCreateInfo layoutCI{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptorSetLayout,
		.pushConstantRangeCount = ncast<uint32_t>(pushConstantRanges.count),
		.pPushConstantRanges = pushConstantRanges.data,
	};

	VkPipelineLayout layout{};
	VkResult res{
		vkCreatePipelineLayout(device.logical, &layoutCI, nullptr, &layout)
	};
	ASSERT(res == VK_SUCCESS);
	return layout;
}
