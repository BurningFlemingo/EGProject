#include "Device.h"

#include "Core/PArena.h"
#include "Core/PArray.h"
#include "Core/PContainer.h"

namespace {
	struct DeviceQueueFamilyIndices {
		pstd::Array<uint32_t, QueueFamily> indices;
		pstd::Array<uint32_t> uniqueIndices;
	};
	DeviceQueueFamilyIndices findDeviceIndices(
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
		const DeviceQueueFamilyIndices& qfi
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

	DeviceQueueFamilyIndices qfi{
		findDeviceIndices(pPersistArena, scratchArena, physicalDevice, surface)
	};

	VkDevice device{ createLogicalDevice(instance, physicalDevice, qfi) };

	auto queues{ pstd::createArray<VkQueue, QueueFamily>(
		pPersistArena, qfi.uniqueIndices.count
	) };

	for (uint32_t i{}; i < queues.count; i++) {
		auto family{ cast<QueueFamily>(i) };
		VkQueue queue{};
		vkGetDeviceQueue(device, qfi.indices[family], 0, &queue);
		queues[family] = queue;
	}

	Device res{ .physical = physicalDevice,
				.logical = device,
				.queueFamilyIndices = qfi.indices,
				.uniqueQueueFamilyIndices = qfi.uniqueIndices,
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

	DeviceQueueFamilyIndices findDeviceIndices(
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

		constexpr uint32_t invalidIndex{ -1u };

		auto indices(pstd::createArray<uint32_t, QueueFamily>(
			pPersistArena, cast<size_t>(QueueFamily::count)
		));

		pstd::fill(&indices, invalidIndex);

		auto uniqueIndices(pstd::createArray<uint32_t>(
			pPersistArena, ncast<size_t>(QueueFamily::count), 0
		));

		for (uint32_t i{}; i < queueFamilyPropCount; i++) {
			VkQueueFamilyProperties familyProps{ queueFamilyProps[i] };
			bool addUniqueIndex{};
			if (familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
				indices[QueueFamily::graphics] == invalidIndex) {
				indices[QueueFamily::graphics] = i;
				addUniqueIndex = true;
			}

			VkBool32 supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(
				physicalDevice, i, surface, &supported
			);

			if (supported &&
				indices[QueueFamily::presentation] == invalidIndex) {
				indices[QueueFamily::presentation] = i;
				addUniqueIndex = true;
			}
			if (addUniqueIndex) {
				pstd::pushBack(&uniqueIndices, i);
			}

			if ((indices[QueueFamily::graphics] != invalidIndex) &&
				(indices[QueueFamily::presentation] != invalidIndex)) {
				break;
			}
		}
		ASSERT(indices[QueueFamily::graphics] != invalidIndex);

		for (uint32_t i{}; i < indices.count; i++) {
			auto family{ (QueueFamily)i };
			if (indices[family] == invalidIndex) {
				indices[family] = indices[QueueFamily::graphics];
			}
		}

		return DeviceQueueFamilyIndices{ .indices = indices,
										 .uniqueIndices = uniqueIndices };
	}

	VkDevice createLogicalDevice(
		VkInstance instance,
		VkPhysicalDevice physicalDevice,
		const DeviceQueueFamilyIndices& qfi
	) {
		const float priorities[1]{ 1.f };

		pstd::StaticArray<
			VkDeviceQueueCreateInfo,
			cast<size_t>(QueueFamily::count)>
			deviceQueueCreateInfos{ .count = 0 };

		for (uint32_t i{}; i < qfi.uniqueIndices.count; i++) {
			VkDeviceQueueCreateInfo deviceQueueCI{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = qfi.uniqueIndices[i],
				.queueCount = 1,
				.pQueuePriorities = priorities,
			};

			pstd::pushBack(&deviceQueueCreateInfos, deviceQueueCI);
		}

		pstd::StaticArray<const char*, 1> deviceExtensions{
			.data = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }
		};

		VkDeviceCreateInfo deviceCI{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfos.count,
			.pQueueCreateInfos = deviceQueueCreateInfos.data,
			.enabledExtensionCount = 1,
			.ppEnabledExtensionNames = deviceExtensions.data
		};

		VkDevice device{};
		VkResult vRes{
			vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device)
		};
		ASSERT(vRes == VK_SUCCESS);
		return device;
	}
}  // namespace
