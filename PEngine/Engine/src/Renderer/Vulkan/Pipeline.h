#pragma once

#include "STD/PArray.h"
#include "Device.h"

#include <vulkan/vulkan.h>

VkPipeline createGraphicsPipeline(
	const Device& device,
	const VkPipelineLayout layout,
	const pstd::Span<VkPipelineShaderStageCreateInfo> shaderStages,
	const pstd::Span<VkFormat> colorFormats
);

VkPipelineLayout createPipelineLayout(
	const Device& device,
	const pstd::Span<VkPushConstantRange> pushConstantRanges,
	const pstd::Span<VkDescriptorSetLayout> descriptorSetLayouts
);
