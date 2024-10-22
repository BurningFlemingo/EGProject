#include "Base.h"
#include <Windows.h>
#include <vulkan/vulkan.h>
#include "Arena.h"
#include "Array.h"

#include "PConsole.h"

#define Max(a, b) a > b ? a : b
#define Min(a, b) a < b ? a : b

enum class InputAction : uint32_t {
	INVALID = 0,
	PRESSED = 1,
	RELEASED = 2,
	REPEATING = 3,

	COUNT
};

enum class InputCode : uint32_t {
	INVALID = 0,
	BACKSPACE = 8,
	TAB = 9,
	ENTER = 27,
	ESC = 27,
	SPACE = 32,

	SHIFT = 128,
	CTRL,
	ALT,

	LEFT_MB,
	RIGHT_MB,
	MIDDLE_MB,

	UP,
	DOWN,
	LEFT,
	RIGHT,

	COUNT
};

struct KeyEvent {
	InputAction action;
	InputCode code;
};

struct WindowData {
	bool isRunning;
	FixedArray<KeyEvent, 1024> eventBuffer;
};

InputCode VirtualToInputCode(const char vcode) {
	if (vcode >= 'A' && vcode <= 'Z' || vcode >= '0' && vcode <= '9') {
		return static_cast<InputCode>(vcode);
	}

	InputCode keyCode{};
	switch (vcode) {
		case VK_BACK:
			keyCode = InputCode::BACKSPACE;
			break;
		case VK_TAB:
			keyCode = InputCode::TAB;
			break;
		case 0x0D:
			keyCode = InputCode::ENTER;
			break;
		case VK_ESCAPE:
			keyCode = InputCode::ESC;
			break;
		case VK_SPACE:
			keyCode = InputCode::SPACE;
			break;
		case VK_SHIFT:
			keyCode = InputCode::SHIFT;
			break;
		case VK_CONTROL:
			keyCode = InputCode::CTRL;
			break;
		case VK_MENU:
			keyCode = InputCode::ALT;
			break;
		case VK_LBUTTON:
			keyCode = InputCode::LEFT_MB;
			break;
		case VK_RBUTTON:
			keyCode = InputCode::RIGHT_MB;
			break;
		case VK_MBUTTON:
			keyCode = InputCode::MIDDLE_MB;
			break;
		case VK_UP:
			keyCode = InputCode::UP;
			break;
		case VK_DOWN:
			keyCode = InputCode::DOWN;
			break;
		case VK_LEFT:
			keyCode = InputCode::LEFT;
			break;
		case VK_RIGHT:
			keyCode = InputCode::RIGHT;
			break;
		default:
			keyCode = InputCode::INVALID;
			break;
	}
	return keyCode;
}

LRESULT CALLBACK
	WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT res{};

	WindowData* windowData{};
	if (uMsg == WM_CREATE) {
		CREATESTRUCT* createStruct{ reinterpret_cast<CREATESTRUCT*>(lParam) };
		windowData =
			reinterpret_cast<WindowData*>(createStruct->lpCreateParams);
		SetWindowLongPtr(
			hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowData)
		);
	} else {
		windowData =
			reinterpret_cast<WindowData*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)
			);
	}

	switch (uMsg) {
		case WM_CLOSE: {
			DestroyWindow(hwnd);
		}
		case WM_DESTROY: {
			windowData->isRunning = false;
		} break;
		case WM_KEYUP: {
			InputCode iCode{ VirtualToInputCode(wParam) };
			InputAction iAction{ InputAction::RELEASED };
			windowData->eventBuffer.pushBack({ .action = iAction,
											   .code = iCode });
		} break;
		case WM_KEYDOWN: {
			InputCode iCode{ VirtualToInputCode(wParam) };
			InputAction iAction{ InputAction::PRESSED };
			windowData->eventBuffer.pushBack({ .action = iAction,
											   .code = iCode });
		} break;
		default: {
			res = DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}

	return res;
}

int main() {
	HINSTANCE hInstance{ GetModuleHandle(0) };

	const char windowClassName[]{ "window class" };

	WNDCLASS windowClass{ .lpfnWndProc = WindowProc,
						  .hInstance = hInstance,
						  .lpszClassName = windowClassName };

	RegisterClass(&windowClass);

	int32_t windowWidth{ 1920 / 2 };
	int32_t windowHeight{ 1080 / 2 };
	bool windowIsFullscreen{ false };

	KeyEvent eventBuffer[1024];
	WindowData windowData{ .isRunning = true };

	LONG windowStyle{};

	LONG adjustedWindowWidth{};
	LONG adjustedWindowHeight{};
	if (windowIsFullscreen) {
		windowStyle = static_cast<LONG>(WS_POPUPWINDOW);
		adjustedWindowWidth = GetSystemMetrics(SM_CXSCREEN);
		adjustedWindowHeight = GetSystemMetrics(SM_CYSCREEN);
	} else {
		windowStyle = WS_OVERLAPPEDWINDOW;
		LONG clientWindowWidth{};
		LONG clientWindowHeight{};

		clientWindowWidth = windowWidth;
		clientWindowHeight = windowHeight;

		RECT clientRect{ .right = clientWindowWidth,
						 .bottom = clientWindowHeight };
		AdjustWindowRectEx(&clientRect, windowStyle, false, 0);

		adjustedWindowWidth = clientRect.right - clientRect.left;
		adjustedWindowHeight = clientRect.bottom - clientRect.top;
	}

	HWND hwnd{ CreateWindowExA(
		0,
		windowClassName,
		"App Name",
		windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		adjustedWindowWidth,
		adjustedWindowHeight,
		0,
		0,
		hInstance,
		&windowData
	) };

	if (hwnd == 0) {
		return 0;
	}

	ShowWindow(hwnd, SW_SHOW);

	// RenderInit

	uint32_t extensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	// need vector

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

	while (windowData.isRunning) {
		MSG msg{};
		while (PeekMessageA(&msg, 0, 0, 0, true) != 0 && windowData.isRunning) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (windowData.eventBuffer.occupancy >= 1) {
			size_t headIndex{ --windowData.eventBuffer.occupancy };
			KeyEvent event{ windowData.eventBuffer[headIndex] };
			if (event.action == InputAction::PRESSED) {
				if (event.code == InputCode::TAB) {
					pstd::consoleWrite("hi world :D");
					windowData.isRunning = false;
				}
			}
		}
	}

	DestroyWindow(hwnd);
}
