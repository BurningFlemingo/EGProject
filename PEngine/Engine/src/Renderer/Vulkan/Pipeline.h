#pragma once

#include "STD/PArray.h"
#include "Device.h"

#include <vulkan/vulkan.h>

VkPipeline createGraphicsPipeline(
	const Device& device,
	const VkPipelineLayout layout,
	const pstd::Array<VkPipelineShaderStageCreateInfo> shaderStages,
	const pstd::Array<VkFormat> colorFormats
);

VkPipelineLayout createPipelineLayout(
	const Device& device,
	const pstd::Array<VkPushConstantRange> pushConstantRanges
);
