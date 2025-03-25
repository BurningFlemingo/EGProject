#include "DebugMessenger.h"
#include "Instance.h"

#include "PContainer.h"
#include "Device.h"
#include "PFileIO.h"
#include "Swapchain.h"
#include "include/Logging.h"

#include "Platforms/VulkanSurface.h"
#include "Renderer/Renderer.h"

#include "Types.h"

#include "PArena.h"
#include "PArray.h"
#include "PString.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <new>

Renderer::State* Renderer::startup(
	pstd::Arena* pArena,
	pstd::LinkedArenaPair scratchArenas,
	const Platform::State& platformState
) {
	VkInstance instance{ createInstance(scratchArenas) };
	VkDebugUtilsMessengerEXT debugMessenger{ createDebugMessenger(instance) };

	VkSurfaceKHR surface{ Platform::createSurface(instance, platformState) };

	Device device{ createDevice(pArena, scratchArenas, instance, surface) };

	// Swapchain swapchain{
	// 	createSwapchain(pArena, scratchArenas, device, surface, platformState)
	// };

	const VkPipelineShaderStageCreateInfo* pStages;
	const VkPipelineVertexInputStateCreateInfo* pVertexInputState;
	const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState;
	const VkPipelineTessellationStateCreateInfo* pTessellationState;
	const VkPipelineViewportStateCreateInfo* pViewportState;
	const VkPipelineRasterizationStateCreateInfo* pRasterizationState;
	const VkPipelineMultisampleStateCreateInfo* pMultisampleState;
	const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState;
	const VkPipelineColorBlendStateCreateInfo* pColorBlendState;
	const VkPipelineDynamicStateCreateInfo* pDynamicState;

	pstd::String fragShaderPath{ pstd::createString("/shaders/first.frag") };
	pstd::String exePath{ pstd::getEXEPath(&scratchArenas.current) };
	fragShaderPath =
		pstd::makeConcatted(&scratchArenas.current, exePath, fragShaderPath);

	pstd::FileHandle fragShaderFile{ pstd::openFile(
		&scratchArenas.current,
		fragShaderPath,
		pstd::FileAccess::read,
		pstd::FileShare::read,
		pstd::FileCreate::openExisting
	) };

	// pstd::Allocation fragShaderAllocation{ pstd::readFile(
	// 	pstd::makeFrame(arenaFrame, arenaFrame.pPersistOffset), fragShaderFile
	// ) };

	// VkShaderModuleCreateInfo fragmentShaderModuleCI{
	// 	.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	// 	.codeSize = fragShaderAllocation.size,
	// 	.pCode = rcast<uint32_t*>(fragShaderAllocation.block)
	// };

	VkShaderModule fragShaderModule{};
	// VkResult res{ vkCreateShaderModule(
	// 	device.logical, &fragmentShaderModuleCI, nullptr, &fragShaderModule
	// ) };
	// ASSERT(res == VK_SUCCESS);

	// pstd::closeFile(fragShaderFile);

	VkGraphicsPipelineCreateInfo gPipeCI{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	};

	// vkDestroyShaderModule(device.logical, fragShaderModule, nullptr);

	pstd::Allocation stateAllocation{ pstd::alloc<State>(pArena) };
	return new (stateAllocation.block) State{ // .swapchain = swapchain,
											  .device = device,
											  .surface = surface,
											  .instance = instance,
											  .debugMessenger = debugMessenger
	};
}

void Renderer::shutdown(State* state) {
	destroySwapchain(&state->swapchain, state->device.logical);
	vkDestroyDevice(state->device.logical, nullptr);
	vkDestroySurfaceKHR(state->instance, state->surface, nullptr);
	destroyDebugMessenger(state->instance, state->debugMessenger);
	vkDestroyInstance(state->instance, nullptr);
}
