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

	uint32_t graphicsQueueFamilyIndex;
	bool graphicsQueueFamilyAvaliable;

	uint32_t presentationQueueFamilyIndex;
	bool presentationQueueFamilyAvaliable;

	for (uint32_t i{}; i < queueFamilyPropCount; i++) {
		VkQueueFamilyProperties familyProps{ queueFamilyProps[i] };
		if (familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
			!graphicsQueueFamilyAvaliable) {
			graphicsQueueFamilyAvaliable = true;
			graphicsQueueFamilyIndex = i;
		}

		VkBool32 supported;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			suitablePhysicalDevice, i, surface, &supported
		);

		if (supported && !presentationQueueFamilyAvaliable) {
			presentationQueueFamilyAvaliable = true;
			presentationQueueFamilyIndex = i;
			break;
		}
	}

	ASSERT(graphicsQueueFamilyIndex);

	const float priorities[1]{ 1.f };

	pstd::BoundedStackArray<VkDeviceQueueCreateInfo, 2>
		deviceQueueCreateInfos{};

	if (graphicsQueueFamilyIndex == presentationQueueFamilyIndex) {
		VkDeviceQueueCreateInfo deviceQueueCI{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = graphicsQueueFamilyIndex,
			.queueCount = 2,
			.pQueuePriorities = priorities,
		};

		pstd::pushBack(&deviceQueueCreateInfos, deviceQueueCI);
	} else {
		VkDeviceQueueCreateInfo deviceQueueCI1{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = graphicsQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = priorities,
		};
		VkDeviceQueueCreateInfo deviceQueueCI2{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = presentationQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = priorities,
		};
		pstd::pushBack(&deviceQueueCreateInfos, deviceQueueCI1);
		pstd::pushBack(&deviceQueueCreateInfos, deviceQueueCI2);
	}

	VkDeviceCreateInfo deviceCI{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfos.count,
		.pQueueCreateInfos = pstd::getData(deviceQueueCreateInfos),
	};

	VkDevice device{};
	vkCreateDevice(suitablePhysicalDevice, &deviceCI, nullptr, &device);

	pstd::Allocation stateAllocation{ pstd::arenaAlloc<State>(stateArena) };

	return new (stateAllocation.block)
		State{ .device = device,
			   .physicalDevice = suitablePhysicalDevice,
			   .surface = surface,
			   .instance = instance,
			   .debugMessenger = debugMessenger };
}

void Renderer::shutdown(State* state) {
	vkDestroyDevice(state->device, nullptr);
	vkDestroySurfaceKHR(state->instance, state->surface, nullptr);
	destroyDebugMessenger(state->instance, state->debugMessenger);
	vkDestroyInstance(state->instance, nullptr);
}
