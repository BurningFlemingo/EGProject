#include "Device.h"
#include "PArena.h"

namespace {
	struct DeviceQueueFamilyIndices {
		pstd::Array<uint32_t, QueueFamily> indices;
		pstd::Array<uint32_t> uniqueIndices;
	};
	DeviceQueueFamilyIndices findDeviceIndices(
		pstd::ArenaFrame&& frame,
		VkPhysicalDevice physicalDevice,
		VkSurfaceKHR surface
	);
	VkPhysicalDevice createPhysicalDevice(
		pstd::ArenaFrame&& arenaFrame, VkInstance instance
	);
	VkDevice createLogicalDevice(
		pstd::ArenaFrame&& arenaFrame,
		VkInstance instance,
		VkPhysicalDevice physicalDevice,
		const DeviceQueueFamilyIndices& qfi
	);
}  // namespace

Device createDevice(
	pstd::ArenaFrame&& arenaFrame, VkInstance instance, VkSurfaceKHR surface
) {
	VkPhysicalDevice physicalDevice{
		createPhysicalDevice({ arenaFrame.pArena, arenaFrame.state }, instance)
	};

	DeviceQueueFamilyIndices qfi{ findDeviceIndices(
		{ .pArena = arenaFrame.pArena, .state = arenaFrame.state },
		physicalDevice,
		surface
	) };
	VkDevice device{ createLogicalDevice(
		{ .pArena = arenaFrame.pArena, .state = arenaFrame.state },
		instance,
		physicalDevice,
		qfi
	) };

	pstd::Array<VkQueue, QueueFamily> queues{
		.allocation = pstd::alloc<VkQueue>(
			&arenaFrame, pstd::getCapacity(qfi.uniqueIndices)
		)
	};
	for (uint32_t i{}; i < pstd::getCapacity(queues); i++) {
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

	DeviceQueueFamilyIndices findDeviceIndices(
		pstd::ArenaFrame&& arenaFrame,
		VkPhysicalDevice physicalDevice,
		VkSurfaceKHR surface
	) {
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
			physicalDevice,
			&queueFamilyPropCount,
			pstd::getData(queueFamilyProps)
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

		for (uint32_t i{}; i < pstd::getCapacity(indices); i++) {
			auto family{ (QueueFamily)i };
			if (indices[family] == invalidIndex) {
				indices[family] = indices[QueueFamily::graphics];
			}
		}

		pstd::Array<uint32_t> uniqueIndicesArray{
			.allocation =
				pstd::alloc<uint32_t>(&arenaFrame, uniqueIndices.count)
		};
		return DeviceQueueFamilyIndices{ .indices = indices,
										 .uniqueIndices = uniqueIndicesArray };
	}

	VkDevice createLogicalDevice(
		pstd::ArenaFrame&& arenaFrame,
		VkInstance instance,
		VkPhysicalDevice physicalDevice,
		const DeviceQueueFamilyIndices& qfi
	) {
		const float priorities[1]{ 1.f };

		pstd::BoundedStaticArray<
			VkDeviceQueueCreateInfo,
			cast<size_t>(QueueFamily::count)>
			deviceQueueCreateInfos{};

		for (uint32_t i{}; i < pstd::getCapacity(qfi.uniqueIndices); i++) {
			VkDeviceQueueCreateInfo deviceQueueCI{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = qfi.uniqueIndices[i],
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
		return device;
	}
}  // namespace
