#pragma once

#include "Window.h"

#include "PString.h"

#include <vulkan/vulkan.h>

namespace Platform {
	VkSurfaceKHR createSurface(VkInstance instance, const State& state);
	const char* getPlatformSurfaceExtension();
	VkExtent2D calcClientExtent(const State& state);
}  // namespace Platform
