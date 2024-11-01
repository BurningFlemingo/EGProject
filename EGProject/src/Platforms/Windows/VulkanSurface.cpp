#define VK_USE_PLATFORM_WIN32_KHR

#include "Platforms/VulkanSurface.h"
#include "Platforms/Windows/Types.h"

#include <vulkan/vulkan_win32.h>

VkSurfaceKHR Platform::createSurface(const State& state) {
	return {};
}

const char* Platform::getPlatformSurfaceExtension() {
	return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}
