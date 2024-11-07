#include <vulkan/vulkan_core.h>
#define VK_USE_PLATFORM_WIN32_KHR

#include "Platforms/VulkanSurface.h"
#include "Platforms/Windows/Types.h"

#include <vulkan/vulkan_win32.h>

VkSurfaceKHR Platform::createSurface(VkInstance instance, const State& state) {
	VkWin32SurfaceCreateInfoKHR surfaceCI{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = state.hInstance,
		.hwnd = state.hwnd,
	};
	VkSurfaceKHR surface{};
	VkResult vRes{
		vkCreateWin32SurfaceKHR(instance, &surfaceCI, nullptr, &surface)
	};
	ASSERT(vRes == VK_SUCCESS);
	return surface;
}

const char* Platform::getPlatformSurfaceExtension() {
	return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}
