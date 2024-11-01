#pragma once

#include <vulkan/vulkan.h>
#include "Window.h"
#include "PString.h"

namespace Platform {
	VkSurfaceKHR createSurface(const State& state);
	const char* getPlatformSurfaceExtension();
}  // namespace Platform
