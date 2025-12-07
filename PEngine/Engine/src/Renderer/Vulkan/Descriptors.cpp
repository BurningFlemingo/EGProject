#include "Descriptors.h"
#include <vulkan/vulkan_core.h>
#include "Device.h"
#include "Types.h"

Descriptors createDescriptorSets(
	pstd::Arena scratchArena,
	Device device,
	pstd::Span<VkDescriptorSetLayoutBinding> bindings,
	uint32_t setCount,
	pstd::Span<VkDescriptorBindingFlags> bindingFlags
) {
	ASSERT(bindingFlags.count == bindings.count);

	VkDescriptorSetLayoutBindingFlagsCreateInfo bindlessSetLayoutSetFlags{
		.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.bindingCount = ncast<uint32_t>(bindingFlags.count),
		.pBindingFlags = bindingFlags.data
	};

	VkDescriptorSetLayoutCreateInfo bindlessSetLayoutCI{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &bindlessSetLayoutSetFlags,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = 1,
		.pBindings = &bindlessDescriptorBinding,
	};
	VkDescriptorSetLayoutCreateInfo uniformBufferSetLayoutCI{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &uniformBufferBinding,
	};

	VkDescriptorSetLayout bindlessSetLayout{};
	VkDescriptorSetLayout uniformBufferSetLayout{};
	vkCreateDescriptorSetLayout(
		device.logical, &bindlessSetLayoutCI, nullptr, &bindlessSetLayout
	);
	vkCreateDescriptorSetLayout(
		device.logical,
		&uniformBufferSetLayoutCI,
		nullptr,
		&uniformBufferSetLayout
	);

	VkDescriptorPoolSize bindlessPoolSize{
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = maxSampledImages
	};

	VkDescriptorPoolSize uniformBufferPoolSize{
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1
	};

	VkDescriptorPoolCreateInfo bindlessPoolCI{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = setCount,
		.poolSizeCount = 1,
		.pPoolSizes = &bindlessPoolSize
	};

	VkDescriptorPool uniformBufferPool{};
	VkDescriptorPool bindlessPool{};
	vkCreateDescriptorPool(
		device.logical, &uniformBufferPoolCI, nullptr, &uniformBufferPool
	);

	vkCreateDescriptorPool(
		device.logical, &bindlessPoolCI, nullptr, &bindlessPool
	);
}
