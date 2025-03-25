#include "Swapchain.h"
#include "PArena.h"
#include "Platforms/VulkanSurface.h"
#include <vulkan/vulkan_core.h>

namespace {
	VkSurfaceFormatKHR findBestFormat(
		pstd::Arena scratchArena, const Device& device, VkSurfaceKHR surface
	);

	VkPresentModeKHR findBestPresentMode(
		pstd::Arena scratchArena, const Device& device, VkSurfaceKHR surface
	);
	VkExtent2D calcSurfaceExtent(
		const VkSurfaceCapabilitiesKHR& surfaceCapabilities,
		const Platform::State& platformState
	);
}  // namespace

Swapchain createSwapchain(
	pstd::Arena* pPersistArena,
	pstd::LinkedArenaPair scratchArenas,
	const Device& device,
	VkSurfaceKHR surface,
	const Platform::State& platformState
) {
	VkSurfaceCapabilitiesKHR surfaceCapabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		device.physical, surface, &surfaceCapabilities
	);

	VkSurfaceFormatKHR format{
		findBestFormat(scratchArenas.current, device, surface)
	};
	VkPresentModeKHR presentMode{
		findBestPresentMode(scratchArenas.current, device, surface)
	};
	VkExtent2D surfaceExtent{
		calcSurfaceExtent(surfaceCapabilities, platformState)
	};

	uint32_t minImageCount{
		min(surfaceCapabilities.minImageCount + 1,
			surfaceCapabilities.maxImageCount - 1
		)  // underflow on purpose for maxImageCount == 0
	};

	VkSwapchainCreateInfoKHR swapchainCI{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = minImageCount,
		.imageFormat = format.format,
		.imageColorSpace = format.colorSpace,
		.imageExtent = surfaceExtent,
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

	uint32_t imageCount{};
	vkGetSwapchainImagesKHR(device.logical, swapchain, &imageCount, nullptr);

	pstd::Array<VkImage> images{
		.allocation = pstd::alloc<VkImage>(pPersistArena, imageCount)
	};

	vkGetSwapchainImagesKHR(
		device.logical, swapchain, &imageCount, pstd::getData(images)
	);

	VkImageViewCreateInfo
		imageViewCI{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					 .viewType = VK_IMAGE_VIEW_TYPE_2D,
					 .format = swapchainCI.imageFormat,
					 .subresourceRange = {
						 .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						 .levelCount = 1,
						 .layerCount = 1,
					 } };

	pstd::Array<VkImageView> imageViews{
		.allocation = pstd::alloc<VkImageView>(pPersistArena, imageCount)
	};
	for (uint32_t i{}; i < imageCount; i++) {
		imageViewCI.image = images[i];
		vkCreateImageView(
			device.logical, &imageViewCI, nullptr, &imageViews[i]
		);
	}

	return Swapchain{ .handle = swapchain,
					  .images = images,
					  .imageViews = imageViews,
					  .createInfo = swapchainCI };
}

void destroySwapchain(Swapchain* swapchain, VkDevice device) {
	ASSERT(swapchain);
	for (uint32_t i{}; i < pstd::getLength(swapchain->imageViews); i++) {
		vkDestroyImageView(device, swapchain->imageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapchain->handle, nullptr);
}

namespace {
	VkSurfaceFormatKHR findBestFormat(
		pstd::Arena scratchArena, const Device& device, VkSurfaceKHR surface
	) {
		uint32_t formatsCount{};
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device.physical, surface, &formatsCount, nullptr
		);

		pstd::Array<VkSurfaceFormatKHR> formats{
			.allocation = pstd::alloc<VkSurfaceFormatKHR>(&scratchArena)
		};
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device.physical, surface, &formatsCount, pstd::getData(formats)
		);
		VkSurfaceFormatKHR format{};
		for (uint32_t i{}; i < pstd::getLength(formats); i++) {
			VkSurfaceFormatKHR availableFormat{ formats[i] };
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace ==
					VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				format = availableFormat;
				break;
			}
		}
		if (format.format == VK_FORMAT_UNDEFINED) {
			format = formats[0];
		}

		return format;
	}
	VkPresentModeKHR findBestPresentMode(
		pstd::Arena scratchArena, const Device& device, VkSurfaceKHR surface
	) {
		uint32_t presentModesCount{};
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device.physical, surface, &presentModesCount, nullptr
		);

		pstd::Array<VkPresentModeKHR> presentModes{
			.allocation = pstd::alloc<VkPresentModeKHR>(&scratchArena)
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

		return presentMode;
	}
	VkExtent2D calcSurfaceExtent(
		const VkSurfaceCapabilitiesKHR& surfaceCapabilities,
		const Platform::State& platformState
	) {
		VkExtent2D surfaceExtent{};
		if (surfaceCapabilities.maxImageExtent.width != UINT32_MAX) {
			VkExtent2D clientWindowExtent{
				Platform::calcClientExtent(platformState)
			};

			surfaceExtent.width = pstd::clamp(
				surfaceCapabilities.minImageExtent.width,
				surfaceCapabilities.maxImageExtent.width,
				clientWindowExtent.width
			);

			surfaceExtent.height = pstd::clamp(
				surfaceCapabilities.minImageExtent.height,
				surfaceCapabilities.maxImageExtent.height,
				clientWindowExtent.height
			);

		} else {
			surfaceExtent = surfaceCapabilities.currentExtent;
		}
		return surfaceExtent;
	}
}  // namespace
