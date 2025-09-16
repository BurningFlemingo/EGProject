#pragma once
#include "Core/PArray.h"
#include "Core/PArray.h"
#include "Core/PFunction.h"

#include "Swapchain.h"
#include "Device.h"

#include <vulkan/vulkan.h>

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
