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

	auto fragShaderPath{ pstd::createString("shaders\\first.frag.spv") };

	pstd::FileHandle fragShaderFile{ pstd::openFile(
		&scratchArena,
		fragShaderPath,
		pstd::FileAccess::read,
		pstd::FileShare::read,
		pstd::FileCreate::openExisting
	) };

	pstd::String fragShaderString{
		pstd::readFile(pPersistArena, fragShaderFile)
	};

	pstd::closeFile(fragShaderFile);

	VkShaderModuleCreateInfo fragmentShaderModuleCI{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = fragShaderString.size,
		.pCode = rcast<const uint32_t*>(fragShaderString.buffer)
	};

	VkShaderModule fragShaderModule{};
	VkResult res{ vkCreateShaderModule(
		device.logical, &fragmentShaderModuleCI, nullptr, &fragShaderModule
	) };

	VkGraphicsPipelineCreateInfo pipelineCI{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	};

	ASSERT(res == VK_SUCCESS);

	VkGraphicsPipelineCreateInfo gPipeCI{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	};

	vkDestroyShaderModule(device.logical, fragShaderModule, nullptr);

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
