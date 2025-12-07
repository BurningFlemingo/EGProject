#pragma once
#include <vulkan/vulkan.h>
#include "STD/PArray.h"

struct Descriptors {
	VkDescriptorSetLayout layout;
	VkDescriptorPool pool;
	pstd::Array<VkDescriptorSet> set;
};

Descriptors createDescriptorSets();
