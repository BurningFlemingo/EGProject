#include "Device.h"
#include "PArena.h"

namespace {
	VkPhysicalDevice createPhysicalDevice(
		pstd::ArenaFrame&& arenaFrame, VkInstance instance
	);
}

Device createDevice(
	pstd::ArenaFrame&& arenaFrame, VkInstance instance, VkSurfaceKHR surface
) {
	VkPhysicalDevice physicalDevice{
		createPhysicalDevice({ arenaFrame.pArena, arenaFrame.state }, instance)
	};

	uint32_t queueFamilyPropCount{};
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice, &queueFamilyPropCount, nullptr
	);

	pstd::Array<VkQueueFamilyProperties> queueFamilyProps{
		.allocation = pstd::scratchAlloc<VkQueueFamilyProperties>(
			&arenaFrame, queueFamilyPropCount
		)
	};
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice, &queueFamilyPropCount, pstd::getData(queueFamilyProps)
	);

	constexpr uint32_t invalidIndex{ -1u };
	pstd::Array<uint32_t, QueueFamily> indices{
		.allocation =
			pstd::alloc<uint32_t>(&arenaFrame, (size_t)QueueFamily::count),
	};
	pstd::fill(&indices, invalidIndex);

	pstd::BoundedStaticArray<uint32_t, cast<size_t>(QueueFamily::count)>
		uniqueIndices{};

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

		if (supported && indices[QueueFamily::presentation] == invalidIndex) {
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

	for (uint32_t i{}; i < pstd::getCapacity(indices); i++) {
		auto family{ (QueueFamily)i };
		if (indices[family] == invalidIndex) {
			indices[family] = indices[QueueFamily::graphics];
		}
	}

	const float priorities[1]{ 1.f };

	pstd::BoundedArray<VkDeviceQueueCreateInfo> deviceQueueCreateInfos{
		.allocation = pstd::scratchAlloc<VkDeviceQueueCreateInfo>(
			&arenaFrame, uniqueIndices.count
		)
	};

	for (uint32_t i{}; i < uniqueIndices.count; i++) {
		VkDeviceQueueCreateInfo deviceQueueCI{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = uniqueIndices[i],
			.queueCount = 1,
			.pQueuePriorities = priorities,
		};

		pstd::pushBack(&deviceQueueCreateInfos, deviceQueueCI);
	}

	VkDeviceCreateInfo deviceCI{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfos.count,
		.pQueueCreateInfos = pstd::getData(deviceQueueCreateInfos),
	};

	VkDevice device{};
	VkResult vRes{
		vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device)
	};
	ASSERT(vRes == VK_SUCCESS);

	pstd::Array<VkQueue, QueueFamily> queues{
		.allocation = pstd::alloc<VkQueue>(&arenaFrame, uniqueIndices.count)
	};
	for (uint32_t i{}; i < pstd::getCapacity(queues); i++) {
		auto family{ (QueueFamily)i };
		VkQueue queue{};
		vkGetDeviceQueue(device, indices[family], 0, &queue);
		queues[family] = queue;
	}

	Device res{
		.physical = physicalDevice,
		.logical = device,
		.queueFamilyIndices = indices,	// allocated in deviceArena
		.queues = queues  // allocated in deviceArena
	};
	return res;
}

namespace {
	VkPhysicalDevice createPhysicalDevice(
		pstd::ArenaFrame&& arenaFrame, VkInstance instance
	) {
		uint32_t physicalDeviceCount{};
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

		pstd::Array<VkPhysicalDevice> physicalDevices{
			.allocation =
				pstd::alloc<VkPhysicalDevice>(&arenaFrame, physicalDeviceCount)
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
		return suitablePhysicalDevice;
	}
}  // namespace
