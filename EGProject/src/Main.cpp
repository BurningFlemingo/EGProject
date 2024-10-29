#include "Base.h"
#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "Logging.h"
#include "PArena.h"
#include "PArray.h"
#include "PCircularBuffer.h"

#include "PConsole.h"
#include "PMatrix.h"
#include "PMemory.h"
#include "PString.h"
#include "PVector.h"
#include "PMath.h"
#include "PFileIO.h"

#include "Platforms/Window.h"
#include "Logging.h"

int main() {
	// TODO: for possible alignment errors, find a better solution
	size_t allocPadding{ 100 };
	size_t subsystemAllocSize{ Platform::getSubsystemAllocSize() };
	size_t scratchSize{ 1024 };
	size_t vulkanInitializationSize{ 1024 };
	size_t totalApplicationSize{ allocPadding + subsystemAllocSize +
								 scratchSize + vulkanInitializationSize };

	pstd::FixedArena applicationArena{
		pstd::allocateFixedArena(totalApplicationSize)
	};

	Platform::State platformState{
		Platform::startup(&applicationArena, "window", 1920 / 2, 1080 / 2)
	};

	uint32_t extensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	pstd::Allocation extensionPropsAllocation{
		pstd::bufferAlloc<VkExtensionProperties>(
			&applicationArena, extensionCount
		)
	};
	vkEnumerateInstanceExtensionProperties(
		nullptr,
		&extensionCount,
		(VkExtensionProperties*)extensionPropsAllocation.block
	);

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
		.allocation{ pstd::bufferAlloc<int>(&applicationArena, 5) },
	};

	pstd::Vec2 myVec1{ .x = 1, .y = 2 };
	pstd::Vec2 myVec2{ .x = 2, .y = 3 };
	pstd::Vec2 vec{ myVec1 + myVec2 };
	pstd::Mat4 myMat{ pstd::getIdentityMatrix<4>() };

	pstd::Rot3 rotor{ pstd::calcRotor(pstd::UP, pstd::RIGHT, pstd::HALF_PI) };

	pstd::Vec3 myVec{ .y = 1.f, .z = 1.f };

	myVec = pstd::calcRotated(myVec, rotor);

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

	volatile float test1{ pstd::atanf(0) };
	test1 = pstd::atanf(1);
	test1 = pstd::atanf(-1);
	test1 = pstd::atanf(10000);
	test1 = pstd::atanf(-10000);
	test1 = pstd::atanf(0.00001);

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
	pstd::freeFixedArena(&applicationArena);
	Platform::shutdown(platformState);
}
