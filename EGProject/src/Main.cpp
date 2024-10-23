#include "Base.h"
#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "PArena.h"
#include "PArray.h"

#include "PConsole.h"
#include "PString.h"

#include "Platforms/Window.h"

int main() {
	// TODO: for alignment errors, find a better solution
	size_t allocPadding{ 100 };
	size_t systemAllocSize{ Platform::getPlatformAllocSize() + allocPadding };
	pstd::FixedArena systemArena{ pstd::allocateFixedArena(systemAllocSize) };
	Platform::State platformState{
		Platform::startup(&systemArena, "uru", 1920 / 2, 1080 / 2)
	};

	pstd::FixedArray<KeyEvent> eventBuffer{
		Platform::getKeyEventBuffer(platformState)
	};

	pstd::FixedArena vulkanArena{ pstd::allocateFixedArena(1024) };
	pstd::saveArenaOffset(&vulkanArena);

	uint32_t extensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	pstd::Allocation<VkExtensionProperties> extensionPropsAllocation{
		pstd::fixedArenaAlloc<VkExtensionProperties>(
			&vulkanArena, extensionCount
		)
	};
	vkEnumerateInstanceExtensionProperties(
		nullptr, &extensionCount, extensionPropsAllocation.block
	);
	pstd::restoreArenaOffset(&vulkanArena);

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
					pstd::Allocation<char> buffer{
						pstd::fixedArenaAlloc<char>(&vulkanArena, 100)
					};
					pstd::consoleWrite(pstd::uint32_tToString(buffer, 123));
					isRunning = false;
				}
			}
		}
	}
	Platform::shutdown(platformState);
}
