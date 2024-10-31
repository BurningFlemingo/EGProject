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

	pstd::BoundedArray<pstd::String, 2> requiredExtensions{
		.staticArray = { Platform::getPlatformSurfaceExtension(),
						 VK_KHR_SURFACE_EXTENSION_NAME },
		.count = 2
	};

	for (int i{}; i < extensionCount; i++) {
		if (requiredExtensions.count == 0) {
			break;
		}
		pstd::String avaliableName{
			pstd::createString(extensionProps[i].extensionName)
		};
		size_t foundIndex{};
		auto matchFunction{ [&](const pstd::String& requiredExtension) {
			return pstd::stringsMatch(requiredExtension, avaliableName);
		} };

		if (pstd::find(requiredExtensions, matchFunction, &foundIndex)) {
			pstd::compactRemove(&requiredExtensions, foundIndex);
		}
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
	ASSERT(res != VK_SUCCESS);

	pstd::Allocation stateAllocation{
		pstd::arenaAlloc<Internal::State>(stateArena)
	};

	return new (stateAllocation.block) Internal::State{ .instance = instance };
}

void Renderer::shutdown(State pState) {
	auto state{ (Internal::State*)pState };

	vkDestroyInstance(state->instance, nullptr);
}
