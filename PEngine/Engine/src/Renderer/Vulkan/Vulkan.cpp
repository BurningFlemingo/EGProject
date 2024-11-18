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
	pstd::ArenaFrame&& arenaFrame, const Platform::State& platformState
) {
	VkInstance instance{ createInstance(pstd::makeFlipped(pstd::ArenaFrame{
		arenaFrame.pArena, arenaFrame.state })) };
	VkDebugUtilsMessengerEXT debugMessenger{ createDebugMessenger(instance) };

	VkSurfaceKHR surface{ Platform::createSurface(instance, platformState) };

	Device device{ createDevice(
		pstd::makeFlipped({ arenaFrame.pArena, arenaFrame.state }),
		instance,
		surface
	) };

	pstd::Allocation stateAllocation{ pstd::alloc<State>(&arenaFrame) };

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
