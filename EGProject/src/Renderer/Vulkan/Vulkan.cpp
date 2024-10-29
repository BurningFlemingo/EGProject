#include "Platforms/VulkanSurface.h"

#include "Renderer/Renderer.h"
#include "PArray.h"
#include <vulkan/vulkan.h>
#include <new>
#include <vulkan/vulkan_core.h>
#include "Types.h"
#include "Logging.h"
#include "PString.h"

Renderer::State Renderer::startup(
	pstd::FixedArena* stateArena, pstd::FixedArena scratchArena
) {
	uint32_t extensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	pstd::FixedArray<VkExtensionProperties> extensionProps{
		.allocation = pstd::arenaAlloc<VkExtensionProperties>(
			&scratchArena, extensionCount
		),
	};
	vkEnumerateInstanceExtensionProperties(
		nullptr,
		&extensionCount,
		(VkExtensionProperties*)extensionProps.allocation.block
	);

	constexpr size_t requiredExtensionCount{ 2 };
	pstd::FixedArray<pstd::String, requiredExtensionCount> arr{
		.staticArray = { Platform::getPlatformSurfaceExtension(),
						 VK_KHR_SURFACE_EXTENSION_NAME }
	};

	for (int i{}; i < pstd::getCapacity(extensionProps); i++) {
		LOG_INFO(extensionProps[i].extensionName);
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

	pstd::Allocation stateAllocation{ pstd::arenaAlloc<InternalState>(stateArena
	) };
	new (stateAllocation.block) InternalState{ .instance = instance };
	return stateAllocation.block;
}

void Renderer::shutdown(State pState) {
	auto state{ (InternalState*)pState };

	vkDestroyInstance(state->instance, nullptr);
}
