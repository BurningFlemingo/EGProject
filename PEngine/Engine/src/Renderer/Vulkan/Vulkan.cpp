#include "DebugMessenger.h"
#include "Instance.h"

#include "PContainer.h"
#include "Renderer/Vulkan/Device.h"
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
	pstd::ArenaFrame&& arenaFrame, const Platform::State& platformState
) {
	VkInstance instance{
		createInstance(pstd::makeFrame(arenaFrame, &arenaFrame.scratchOffset))
	};
	VkDebugUtilsMessengerEXT debugMessenger{ createDebugMessenger(instance) };

	VkSurfaceKHR surface{ Platform::createSurface(instance, platformState) };

	Device device{ createDevice(
		pstd::makeFrame(arenaFrame, arenaFrame.pPersistOffset),
		instance,
		surface
	) };

	uint32_t formatsCount{};
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		device.physical, surface, &formatsCount, nullptr
	);

	pstd::Array<VkSurfaceFormatKHR> formats{
		.allocation = pstd::scratchAlloc<VkSurfaceFormatKHR>(&arenaFrame)
	};
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		device.physical, surface, &formatsCount, pstd::getData(formats)
	);
	VkSurfaceFormatKHR format{};
	for (uint32_t i{}; i < pstd::getLength(formats); i++) {
		VkSurfaceFormatKHR availableFormat{ formats[i] };
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			format = availableFormat;
			break;
		}
	}
	if (format.format == VK_FORMAT_UNDEFINED) {
		format = formats[0];
	}

	uint32_t presentModesCount{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		device.physical, surface, &presentModesCount, nullptr
	);

	pstd::Array<VkPresentModeKHR> presentModes{
		.allocation = pstd::scratchAlloc<VkPresentModeKHR>(&arenaFrame)
	};
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		device.physical,
		surface,
		&presentModesCount,
		pstd::getData(presentModes)
	);

	VkPresentModeKHR presentMode{ VK_PRESENT_MODE_FIFO_KHR };
	for (uint32_t i{}; i < pstd::getLength(presentModes); i++) {
		VkPresentModeKHR availablePresentMode{ presentModes[i] };
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentMode = availablePresentMode;
			break;
		}
	}

	VkSurfaceCapabilitiesKHR surfaceCapabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		device.physical, surface, &surfaceCapabilities
	);

	VkExtent2D swapchainImageExtent{};
	if (surfaceCapabilities.maxImageExtent.width != UINT32_MAX) {
		VkExtent2D clientWindowExtent{ Platform::calcClientExtent(platformState
		) };

		swapchainImageExtent.width = pstd::clamp(
			surfaceCapabilities.minImageExtent.width,
			surfaceCapabilities.maxImageExtent.width,
			clientWindowExtent.width
		);

		swapchainImageExtent.height = pstd::clamp(
			surfaceCapabilities.minImageExtent.height,
			surfaceCapabilities.maxImageExtent.height,
			clientWindowExtent.height
		);

	} else {
		swapchainImageExtent = surfaceCapabilities.currentExtent;
	}

	uint32_t imageCount{
		min(surfaceCapabilities.minImageCount + 1,
			surfaceCapabilities.maxImageCount - 1
		)  // underflow on purpose for maxImageCount == 0
	};

	VkSwapchainCreateInfoKHR swapchainCI{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = format.format,
		.imageColorSpace = format.colorSpace,
		.imageExtent = swapchainImageExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = surfaceCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
	};
	VkSwapchainKHR swapchain{};
	VkResult res{
		vkCreateSwapchainKHR(device.logical, &swapchainCI, nullptr, &swapchain)
	};
	ASSERT(res == VK_SUCCESS);

	pstd::Allocation stateAllocation{ pstd::alloc<State>(&arenaFrame) };
	return new (stateAllocation.block)
		State{ .swapchain = swapchain,
			   .device = device,
			   .surface = surface,
			   .instance = instance,
			   .debugMessenger = debugMessenger };
}

void Renderer::shutdown(State* state) {
	vkDestroySwapchainKHR(state->device.logical, state->swapchain, nullptr);

	vkDestroyDevice(state->device.logical, nullptr);
	vkDestroySurfaceKHR(state->instance, state->surface, nullptr);
	destroyDebugMessenger(state->instance, state->debugMessenger);
	vkDestroyInstance(state->instance, nullptr);
}
