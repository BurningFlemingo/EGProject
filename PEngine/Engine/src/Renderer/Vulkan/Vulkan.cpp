#include "DebugMessenger.h"
#include "Instance.h"

#include "PContainer.h"
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

namespace {
	enum class QueueFamily : uint32_t { graphics = 0, presentation = 1, count };
}  // namespace

Renderer::State* Renderer::startup(
	pstd::FixedArena* stateArena,
	pstd::FixedArena scratchArena,
	const Platform::State& platformState
) {
	VkInstance instance{ createInstance(scratchArena) };
	VkDebugUtilsMessengerEXT debugMessenger{ createDebugMessenger(instance) };

	VkSurfaceKHR surface{ Platform::createSurface(instance, platformState) };

	Device device{ createDevice(stateArena, scratchArena, instance, surface) };

	pstd::Allocation stateAllocation{ pstd::arenaAlloc<State>(stateArena) };

	return new (stateAllocation.block)
		State{ .device = device,
			   .surface = surface,
			   .instance = instance,
			   .debugMessenger = debugMessenger };
}

void Renderer::shutdown(State* state) {
	vkDestroyDevice(state->device.logical, nullptr);
	vkDestroySurfaceKHR(state->instance, state->surface, nullptr);
	destroyDebugMessenger(state->instance, state->debugMessenger);
	vkDestroyInstance(state->instance, nullptr);
}
