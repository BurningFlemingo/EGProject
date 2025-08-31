#pragma once
#include <vulkan/vulkan.h>
#include "PArray.h"
#include "Device.h"
#include "PArray.h"
#include "PFunction.h"

#include "Swapchain.h"

namespace Renderer {
	struct State {
		Swapchain swapchain;
		Device device;
		VkSurfaceKHR surface;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;

		pstd::DArray<pstd::Delegate<void()>*> deleters;
	};
}  // namespace Renderer
