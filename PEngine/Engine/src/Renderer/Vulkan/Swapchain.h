#pragma once
#include "PArray.h"
#include "PArena.h"

#include "Device.h"
#include "Platforms/Window.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct Swapchain {
	VkSwapchainKHR handle;
	pstd::Array<VkImage> images;
	pstd::Array<VkImageView> imageViews;
	VkSwapchainCreateInfoKHR createInfo;
};

Swapchain createSwapchain(
	pstd::Arena* pPersistArena,
	pstd::LinkedArenaPair scratchArenas,
	const Device& device,
	VkSurfaceKHR surface,
	const Platform::State& platformState
);

void destroySwapchain(Swapchain* swapchain, VkDevice device);
