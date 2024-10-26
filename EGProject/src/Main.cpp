#include "Base.h"
#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "PArena.h"
#include "PArray.h"
#include "PCircularBuffer.h"

#include "PConsole.h"
#include "PMemory.h"
#include "PString.h"
#include "PVector.h"
#include "PMath.h"

#include "Platforms/Window.h"

int main() {
	// TODO: for possible alignment errors, find a better solution
	size_t allocPadding{ 100 };
	size_t subsystemAllocSize{ Platform::getSubsystemAllocSize() };

	pstd::FixedArena subsystemArena{ .allocation =
										 pstd::heapAlloc(subsystemAllocSize) };
	Platform::State platformState{
		Platform::startup(&subsystemArena, "window", 1920 / 2, 1080 / 2)
	};

	pstd::FixedArena vulkanArena{ .allocation = pstd::heapAlloc(1024) };

	uint32_t extensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	pstd::Allocation extensionPropsAllocation{
		pstd::bufferAlloc<VkExtensionProperties>(&vulkanArena, extensionCount)
	};
	vkEnumerateInstanceExtensionProperties(
		nullptr,
		&extensionCount,
		(VkExtensionProperties*)extensionPropsAllocation.block
	);
	pstd::reset(&vulkanArena);

	pstd::heapFree(&vulkanArena.allocation);

	VkInstance instance{};

	VkApplicationInfo appInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
							   .pApplicationName = "APPNAME",
							   .applicationVersion =
								   VK_MAKE_API_VERSION(0, 1, 0, 0),
							   .pEngineName = "NA",
							   .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
							   .apiVersion = VK_API_VERSION_1_0 };

	VkInstanceCreateInfo vkInstanceCI{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
	};
	VkResult res{ vkCreateInstance(&vkInstanceCI, nullptr, &instance) };
	if (res != VK_SUCCESS) {
		return 0;
	}
	vkDestroyInstance(instance, nullptr);
	instance = {};

	pstd::CircularBuffer<int> cBuf{
		.allocation{ pstd::heapAlloc<int>(4) },
	};

	pstd::Allocation msgBuffer{ pstd::heapAlloc(100) };

	pstd::v2<int> myVec1{ .x = 1, .y = 2 };
	pstd::v2<int> myVec2{ .x = 2, .y = 3 };
	pstd::v2<int> vec{ myVec1 + myVec2 };

	pstd::consoleWrite(pstd::uint32_tToString(msgBuffer, vec.x));
	pstd::consoleWrite(pstd::uint32_tToString(msgBuffer, vec.y));

	pstd::pushBack(&cBuf, 1);
	pstd::pushBack(&cBuf, 2);
	pstd::pushBack(&cBuf, 3);
	pstd::pushBack(&cBuf, 4);
	pstd::pushBack(&cBuf, 5);
	pstd::pushBack(&cBuf, 6);

	int var{ pstd::indexRead(cBuf, 0) };
	uint32_t offset{ (uint32_t)cBuf.headIndex };
	pstd::consoleWrite("headOffset: ");
	pstd::consoleWrite(pstd::uint32_tToString(msgBuffer, offset));

	volatile float test1{ pstd::sqrtfNewton(9) };
	test1 = pstd::sqrtfNewton(8);
	test1 = pstd::sqrtfNewton(2);

	bool isRunning{ true };
	while (isRunning && Platform::isRunning(platformState)) {
		MSG msg{};
		while (PeekMessageA(&msg, 0, 0, 0, true) != 0 && isRunning) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		pstd::FixedArray<KeyEvent> eventBuffer{
			Platform::popKeyEvents(platformState)
		};

		for (int i{}; i < eventBuffer.count; i++) {
			KeyEvent event{ pstd::indexRead(eventBuffer, i) };
			if (event.action == InputAction::PRESSED) {
				if (event.code == InputCode::TAB) {
					// pstd::consoleWrite(pstd::uint32_tToString(buffer, 123));
					isRunning = false;
				}
			}
		}
	}
	pstd::heapFree(&vulkanArena.allocation);
	pstd::heapFree(&cBuf.allocation);
	Platform::shutdown(platformState);
}
