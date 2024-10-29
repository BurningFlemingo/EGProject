#pragma once

#include <vulkan/vulkan.h>
#include "Window.h"
#include "PString.h"

namespace Platform {
	VkSurfaceKHR createSurface(const State& state);
	pstd::String getPlatformSurfaceExtension();
}  // namespace Platform
