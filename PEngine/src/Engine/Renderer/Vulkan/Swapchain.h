#pragma once
#include "Core/PArray.h"
#include "Core/PArena.h"
#include "Engine/Platforms/Window.h"

#include "Device.h"

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
	pstd::Arena scratchArena,
	const Device& device,
	VkSurfaceKHR surface,
	const Platform::State& platformState
);

void destroySwapchain(Swapchain* swapchain, VkDevice device);
