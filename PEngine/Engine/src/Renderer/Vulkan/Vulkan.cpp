#include "DebugMessenger.h"
#include "Instance.h"

#include "include/Logging.h"

#include "Platforms/VulkanSurface.h"
#include "Renderer/Renderer.h"

#include "Types.h"

#include "PArena.h"
#include "PArray.h"
#include "PString.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <new>

Renderer::State* Renderer::startup(
	pstd::FixedArena* stateArena, pstd::FixedArena scratchArena
) {
	VkInstance instance{ createInstance(scratchArena) };
	VkDebugUtilsMessengerEXT debugMessenger{ createDebugMessenger(instance) };

	pstd::Allocation stateAllocation{ pstd::arenaAlloc<State>(stateArena) };

	return new (stateAllocation.block)
		State{ .instance = instance, .debugMessenger = debugMessenger };
}

void Renderer::shutdown(State* state) {
	destroyDebugMessenger(state->instance, state->debugMessenger);
	vkDestroyInstance(state->instance, nullptr);
}
