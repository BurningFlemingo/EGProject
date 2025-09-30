#include "Device.h"

#include "Core/PArena.h"
#include "Core/PArray.h"
#include "Core/PContainer.h"
#include <vulkan/vulkan_core.h>

namespace {
	pstd::Array<uint32_t, QueueFamily> findDeviceIndices(
		pstd::Arena* pPersistArena,
		pstd::Arena scratchArena,
		VkPhysicalDevice physicalDevice,
		VkSurfaceKHR surface
	);
	VkPhysicalDevice
		createPhysicalDevice(pstd::Arena scratchArena, VkInstance instance);
	VkDevice createLogicalDevice(
		VkInstance instance,
		VkPhysicalDevice physicalDevice,
		const pstd::Array<uint32_t, QueueFamily>& queueFamilyIndices
	);
}  // namespace

Device createDevice(
	pstd::Arena* pPersistArena,
	pstd::Arena scratchArena,
	VkInstance instance,
	VkSurfaceKHR surface
) {
	VkPhysicalDevice physicalDevice{
		createPhysicalDevice(scratchArena, instance)
	};

	pstd::Array<uint32_t, QueueFamily> queueFamilyIndices{
		findDeviceIndices(pPersistArena, scratchArena, physicalDevice, surface)
	};

	VkDevice device{
		createLogicalDevice(instance, physicalDevice, queueFamilyIndices)
	};

	auto queues{ pstd::createArray<VkQueue, QueueFamily>(
		pPersistArena, queueFamilyIndices.count
	) };

	for (uint32_t i{}; i < queues.count; i++) {
		auto family{ cast<QueueFamily>(i) };
		VkQueue queue{};
		vkGetDeviceQueue(device, queueFamilyIndices[family], 0, &queue);
		queues[family] = queue;
	}

	Device res{ .physical = physicalDevice,
				.logical = device,
				.queueFamilyIndices = queueFamilyIndices,
				.queues = queues };
	return res;
}

namespace {
	VkPhysicalDevice
		createPhysicalDevice(pstd::Arena scratchArena, VkInstance instance) {
		uint32_t physicalDeviceCount{};
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

		auto physicalDevices{ pstd::createArray<VkPhysicalDevice>(
			&scratchArena, physicalDeviceCount
		) };

		vkEnumeratePhysicalDevices(
			instance, &physicalDeviceCount, physicalDevices.data
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
		return suitablePhysicalDevice;
	}

	pstd::Array<uint32_t, QueueFamily> findDeviceIndices(
		pstd::Arena* pPersistArena,
		pstd::Arena scratchArena,
		VkPhysicalDevice physicalDevice,
		VkSurfaceKHR surface
	) {
		uint32_t queueFamilyPropCount{};
		vkGetPhysicalDeviceQueueFamilyProperties(
			physicalDevice, &queueFamilyPropCount, nullptr
		);

		auto queueFamilyProps{ pstd::createArray<VkQueueFamilyProperties>(
			&scratchArena, queueFamilyPropCount
		) };

		vkGetPhysicalDeviceQueueFamilyProperties(
			physicalDevice, &queueFamilyPropCount, queueFamilyProps.data
		);

		auto indices(pstd::createArray<uint32_t, QueueFamily>(
			pPersistArena, cast<size_t>(QueueFamily::count)
		));

		constexpr uint32_t invalidIndex{ -1u };
		pstd::fill(&indices, invalidIndex);

		auto nQueueFamilies{ ncast<int>(QueueFamily::count) };

		for (uint32_t i{}; i < queueFamilyPropCount; i++) {
			VkQueueFamilyProperties familyProps{ queueFamilyProps[i] };
			if (familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
				indices[QueueFamily::graphics] == invalidIndex) {
				indices[QueueFamily::graphics] = i;
				nQueueFamilies--;
			}

			VkBool32 surfaceSupported;
			vkGetPhysicalDeviceSurfaceSupportKHR(
				physicalDevice, i, surface, &surfaceSupported
			);

			if (surfaceSupported &&
				indices[QueueFamily::presentation] == invalidIndex) {
				indices[QueueFamily::presentation] = i;
				nQueueFamilies--;
			}

			if (nQueueFamilies <= 0) {
				break;
			}
		}

		ASSERT(indices[QueueFamily::graphics] != invalidIndex);

		if (nQueueFamilies <= 0) {
			for (uint32_t i{}; i < indices.count; i++) {
				auto family{ ncast<QueueFamily>(i) };
				if (indices[family] == invalidIndex) {
					indices[family] = indices[QueueFamily::graphics];
				}
			}
		}

		return indices;
	}

	VkDevice createLogicalDevice(
		VkInstance instance,
		VkPhysicalDevice physicalDevice,
		const pstd::Array<uint32_t, QueueFamily>& queueFamilyIndices
	) {
		const float priorities[1]{ 1.f };

		VkDeviceQueueCreateInfo
			deviceQueueCIBuffer[cast<size_t>(QueueFamily::count)]{};

		auto deviceQueueCreateInfos{
			pstd::createArray<VkDeviceQueueCreateInfo>(deviceQueueCIBuffer, 0)
		};

		for (uint32_t i{}; i < queueFamilyIndices.count; i++) {
			auto index{ queueFamilyIndices[ncast<QueueFamily>(i)] };
			bool repeatedIndex{};
			for (uint32_t j{}; j < deviceQueueCreateInfos.count; j++) {
				if (deviceQueueCreateInfos[j].queueFamilyIndex == index) {
					repeatedIndex = true;
					break;
				}
			}
			if (repeatedIndex) {
				continue;
			}

			VkDeviceQueueCreateInfo deviceQueueCI{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = index,
				.queueCount = 1,
				.pQueuePriorities = priorities,
			};

			pstd::pushBack(&deviceQueueCreateInfos, deviceQueueCI);
		}

		const char* deviceExtensionsBuffer[]{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		auto deviceExtensions{
			pstd::createArray<const char*>(deviceExtensionsBuffer)
		};

		// TODO: move this into argument and check device actually has these
		// features
		VkPhysicalDeviceVulkan13Features features13{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.dynamicRendering = VK_TRUE
		};

		VkPhysicalDeviceFeatures2 physicalDeviceFeatures{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &features13
		};

		VkDeviceCreateInfo deviceCI{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &physicalDeviceFeatures,
			.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfos.count,
			.pQueueCreateInfos = deviceQueueCreateInfos.data,
			.enabledExtensionCount = 1,
			.ppEnabledExtensionNames = deviceExtensions.data,
		};

		VkDevice device{};
		VkResult vRes{
			vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device)
		};
		ASSERT(vRes == VK_SUCCESS);
		return device;
	}
}  // namespace
