#include "Renderer/Renderer.h"
#include "PArray.h"
#include <vulkan/vulkan.h>
#include <new>
#include <vulkan/vulkan_core.h>
#include "StateImpl.h"
#include "Logging.h"

Renderer::State Renderer::startup(
	pstd::FixedArena* stateArena, pstd::FixedArena scratchArena
) {
	uint32_t extensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	pstd::FixedArray<VkExtensionProperties> extensionProps{
		.allocation = pstd::arenaAlloc<VkExtensionProperties>(
			&scratchArena, extensionCount
		),
		.count = extensionCount
	};
	vkEnumerateInstanceExtensionProperties(
		nullptr,
		&extensionCount,
		(VkExtensionProperties*)extensionProps.allocation.block
	);

	for (int i{}; i < extensionProps.count; i++) {
		LOG_INFO(pstd::indexRead(extensionProps, i).extensionName);
	}

	VkInstance instance{};
	VkApplicationInfo appInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
							   .pApplicationName = "APPNAME",
							   .applicationVersion =
								   VK_MAKE_API_VERSION(0, 1, 0, 0),
							   .pEngineName = "NA",
							   .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
							   .apiVersion = VK_API_VERSION_1_0 };

	VkInstanceCreateInfo vkInstanceCI{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
	};
	VkResult res{ vkCreateInstance(&vkInstanceCI, nullptr, &instance) };
	if (res != VK_SUCCESS) {
		return {};
	}

	pstd::Allocation stateAllocation{ pstd::arenaAlloc<StateImpl>(stateArena) };
	new (stateAllocation.block) StateImpl{ .instance = instance };
	return stateAllocation.block;
}

void Renderer::shutdown(State pState) {
	auto state{ (StateImpl*)pState };

	vkDestroyInstance(state->instance, nullptr);
}
