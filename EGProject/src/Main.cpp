#include "Base.h"
#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "Logging.h"
#include "PArena.h"
#include "PArray.h"
#include "PCircularBuffer.h"

#include "PConsole.h"
#include "PMemory.h"
#include "PString.h"
#include "PVector.h"
#include "PMath.h"

#include "Platforms/Window.h"
#include "Logging.h"

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

	pstd::v2<int> myVec1{ .x = 1, .y = 2 };
	pstd::v2<int> myVec2{ .x = 2, .y = 3 };
	pstd::v2<int> vec{ myVec1 + myVec2 };

	pstd::FixedArena msgBuffer{ .allocation = pstd::heapAlloc(100) };

	pstd::rot3 rotor{ pstd::createRotor(pstd::UP, pstd::RIGHT, pstd::HALF_PI) };

	pstd::v3<float> myVec{ .y = 1.f, .z = 1.f };

	myVec = pstd::rotate(rotor, myVec);

	Console::log("x = %f \n", myVec.x);
	Console::log("y = %f \n", myVec.y);
	Console::log("z = %f \n", myVec.z);

	LOG_ERROR("my favorite number is %f cus its cool", -0.3f);

	pstd::pushBack(&cBuf, 1);
	pstd::pushBack(&cBuf, 2);
	pstd::pushBack(&cBuf, 3);
	pstd::pushBack(&cBuf, 4);
	pstd::pushBack(&cBuf, 5);
	pstd::pushBack(&cBuf, 6);

	volatile float test1{ pstd::atanPade(0) };
	test1 = pstd::atanPade(1);
	test1 = pstd::atanPade(-1);
	test1 = pstd::atanPade(10000);
	test1 = pstd::atanPade(-10000);
	test1 = pstd::atanPade(0.00001);

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
					isRunning = false;
				}
			}
		}
	}
	pstd::heapFree(&vulkanArena.allocation);
	pstd::heapFree(&cBuf.allocation);
	Platform::shutdown(platformState);
}
