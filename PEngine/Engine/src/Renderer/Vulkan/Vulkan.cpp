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
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShaderModule,
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

	VkAttachmentReference colorAttachmentRef{ .attachment = 0 };

	VkSubpassDescription subpassDescription{};

	VkGraphicsPipelineCreateInfo pipelineCI{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pStages = shaderStages,
	};

	vkDestroyShaderModule(device.logical, fragShaderModule, nullptr);
	vkDestroyShaderModule(device.logical, vertShaderModule, nullptr);

	State* state{ pstd::alloc<State>(pPersistArena) };
	return new (state) State{ .swapchain = swapchain,
							  .device = device,
							  .surface = surface,
							  .instance = instance,
							  .debugMessenger = debugMessenger };
}

void Renderer::shutdown(State* state) {
	destroySwapchain(&state->swapchain, state->device.logical);
	vkDestroyDevice(state->device.logical, nullptr);
	vkDestroySurfaceKHR(state->instance, state->surface, nullptr);
	destroyDebugMessenger(state->instance, state->debugMessenger);
	vkDestroyInstance(state->instance, nullptr);
}
