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

#include "Platforms/Window.h"

int main() {
	// TODO: for possible alignment errors, find a better solution
	size_t allocPadding{ 100 };
	size_t systemAllocSize{ Platform::getPlatformAllocSize() + allocPadding };
	pstd::FixedArena systemArena{ .allocation =
									  pstd::heapAlloc(systemAllocSize) };
	Platform::State platformState{
		Platform::startup(&systemArena, "uru", 1920 / 2, 1080 / 2)
	};

	pstd::FixedArray<KeyEvent> eventBuffer{
		Platform::getKeyEventBuffer(platformState)
	};

	pstd::FixedArena vulkanArena{ .allocation = pstd::heapAlloc(1024) };
	{
		pstd::FixedArena stack{ vulkanArena };

		uint32_t extensionCount{};
		vkEnumerateInstanceExtensionProperties(
			nullptr, &extensionCount, nullptr
		);
		pstd::Allocation extensionPropsAllocation{
			pstd::bufferAlloc<VkExtensionProperties>(&stack, extensionCount)
		};
		vkEnumerateInstanceExtensionProperties(
			nullptr,
			&extensionCount,
			(VkExtensionProperties*)extensionPropsAllocation.block
		);
	}
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

	pstd::pushBack(&cBuf, 1);
	pstd::pushBack(&cBuf, 2);
	pstd::pushBack(&cBuf, 3);
	pstd::pushBack(&cBuf, 4);
	pstd::pushBack(&cBuf, 5);
	pstd::pushBack(&cBuf, 6);

	pstd::Allocation buffer{ pstd::heapAlloc(100) };

	int var{ pstd::indexRead(cBuf, 0) };
	uint32_t offset{ (uint32_t)cBuf.headIndex };
	pstd::consoleWrite("headOffset: ");
	pstd::consoleWrite(pstd::uint32_tToString(buffer, offset));

	bool isRunning{ true };
	while (isRunning && Platform::isRunning(platformState)) {
		MSG msg{};
		while (PeekMessageA(&msg, 0, 0, 0, true) != 0 && isRunning) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (eventBuffer.occupancy >= 1) {
			size_t headIndex{ --eventBuffer.occupancy };
			KeyEvent event{ eventBuffer[headIndex] };
			if (event.action == InputAction::PRESSED) {
				if (event.code == InputCode::TAB) {
					pstd::consoleWrite(pstd::uint32_tToString(buffer, 123));
					isRunning = false;
				}
			}
		}
	}
	pstd::heapFree(&vulkanArena.allocation);
	pstd::heapFree(&cBuf.allocation);
	Platform::shutdown(platformState);
}
