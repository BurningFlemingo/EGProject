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

namespace {
	enum class QueueFamilyType : uint32_t {
		graphics = 0,
		compute = 1,
		transfer = 2,
		presentation = 3,
		count
	};

	enum class QueueFamilySupported : uint32_t {
		invalid = ~0u,
		graphics = 0b1,
		compute = 0b10,
		transfer = 0b100,
		presentation = 0b1000,
		count
	};

	using QueueFamilyIndicesSupportedBits = uint32_t;
	union QueueFamilyIndices {
		uint32_t indices[4];
		struct {
			uint32_t graphicsFamilyIndex;
			uint32_t computeFamilyIndex;
			uint32_t transferFamilyIndex;
			uint32_t presentationFamilyIndex;
		};
	};
}  // namespace

Renderer::State* Renderer::startup(
	pstd::FixedArena* stateArena,
	pstd::FixedArena scratchArena,
	const Platform::State& platformState
) {
	VkInstance instance{ createInstance(scratchArena) };
	VkDebugUtilsMessengerEXT debugMessenger{ createDebugMessenger(instance) };

	VkSurfaceKHR surface{ Platform::createSurface(instance, platformState) };

	uint32_t physicalDeviceCount{};
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

	pstd::FixedArray<VkPhysicalDevice> physicalDevices{
		.allocation = pstd::arenaAlloc<VkPhysicalDevice>(
			&scratchArena, physicalDeviceCount
		)
	};

	vkEnumeratePhysicalDevices(
		instance, &physicalDeviceCount, pstd::getData(physicalDevices)
	);

	uint32_t maxDeviceScore{};
	VkPhysicalDevice suitablePhysicalDevice{ physicalDevices[0] };
	for (uint32_t i{}; i < physicalDeviceCount; i++) {
		VkPhysicalDevice physicalDevice{ physicalDevices[i] };
		VkPhysicalDeviceProperties physicalDeviceProps{};
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProps);

		uint32_t currentDeviceScore{};
		switch (physicalDeviceProps.deviceType) {
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				currentDeviceScore += 10000;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				currentDeviceScore += 1000;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				currentDeviceScore += 100;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				currentDeviceScore += 10;
				break;
			default:
				break;
		}

		if (currentDeviceScore > maxDeviceScore) {
			maxDeviceScore = currentDeviceScore;
			suitablePhysicalDevice = physicalDevice;
		}
	}

	uint32_t queueFamilyPropCount{};
	vkGetPhysicalDeviceQueueFamilyProperties(
		suitablePhysicalDevice, &queueFamilyPropCount, nullptr
	);

	pstd::FixedArray<VkQueueFamilyProperties> queueFamilyProps{
		.allocation = pstd::arenaAlloc<VkQueueFamilyProperties>(
			&scratchArena, queueFamilyPropCount
		)
	};
	vkGetPhysicalDeviceQueueFamilyProperties(
		suitablePhysicalDevice,
		&queueFamilyPropCount,
		pstd::getData(queueFamilyProps)
	);

	uint32_t familySupportFlags{};
	bool presentationSupport{};

	uint32_t presentationFamilyIndex{};
	uint32_t graphicsFamilyIndex{};
	uint32_t transferFamilyIndex{};
	uint32_t computeFamilyIndex{};

	for (uint32_t i{}; i < queueFamilyPropCount; i++) {
		VkQueueFamilyProperties familyProps{ queueFamilyProps[i] };
		if (familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			familySupportFlags |= VK_QUEUE_GRAPHICS_BIT;
			graphicsFamilyIndex = i;
		}
		if (familyProps.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			familySupportFlags |= VK_QUEUE_TRANSFER_BIT;
			transferFamilyIndex = i;
		}
		if (familyProps.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			familySupportFlags |= VK_QUEUE_COMPUTE_BIT;
			computeFamilyIndex = i;
		}

		VkBool32 supported;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			suitablePhysicalDevice, i, surface, &supported
		);
		if (supported) {
			presentationSupport = true;
			presentationFamilyIndex = i;
		}
	}

	// const float priorities[1]{ 1.f };
	// VkDeviceQueueCreateInfo deviceQueueCI{
	// 	.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
	// 	.queueFamilyIndex = 0,
	// 	.queueCount = 1,
	// 	.pQueuePriorities = priorities,
	// };

	// VkDeviceCreateInfo deviceCI{
	// 	.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	// };
	// vkCreateDevice();

	pstd::Allocation stateAllocation{ pstd::arenaAlloc<State>(stateArena) };

	return new (stateAllocation.block)
		State{ .surface = surface,
			   .instance = instance,
			   .debugMessenger = debugMessenger };
}

void Renderer::shutdown(State* state) {
	vkDestroySurfaceKHR(state->instance, state->surface, nullptr);
	destroyDebugMessenger(state->instance, state->debugMessenger);
	vkDestroyInstance(state->instance, nullptr);
}
