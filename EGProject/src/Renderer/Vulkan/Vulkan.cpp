#include "PArena.h"
#include "Platforms/VulkanSurface.h"

#include "Renderer/Renderer.h"
#include "PArray.h"
#include <vulkan/vulkan.h>
#include <new>
#include <vulkan/vulkan_core.h>
#include "Types.h"
#include "Logging.h"
#include "PString.h"

#include "DebugMessenger.h"
#include "Instance.h"

Renderer::State Renderer::startup(
	pstd::FixedArena* stateArena, pstd::FixedArena scratchArena
) {
	VkInstance instance{ createInstance(scratchArena) };
	VkDebugUtilsMessengerEXT debugMessenger{ createDebugMessenger(instance) };

	pstd::Allocation stateAllocation{
		pstd::arenaAlloc<Internal::State>(stateArena)
	};

	return new (stateAllocation.block
	) Internal::State{ .instance = instance, .debugMessenger = debugMessenger };
}

void Renderer::shutdown(State pState) {
	auto state{ (Internal::State*)pState };

	destroyDebugMessenger(state->instance, state->debugMessenger);
	vkDestroyInstance(state->instance, nullptr);
}
