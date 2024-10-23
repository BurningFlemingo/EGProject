#include "Platforms/Window.h"

#include "PMemory.h"
#include "Events.h"

#include "Base.h"
#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "PArray.h"
#include "PAlgorithm.h"
#include <new>

#include "PConsole.h"
#include "PString.h"

namespace {
	struct WindowData {
		bool isRunning;
		pstd::FixedArray<KeyEvent> eventBuffer;
	};
	struct StateImpl {
		WindowData windowData;
		HWND hwnd;
		HINSTANCE hInstance;
	};

	LRESULT CALLBACK
		windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	InputCode virtualToInputCode(const char vcode);
}  // namespace

size_t Platform::getPlatformAllocSize() {
	return sizeof(StateImpl);
}

Platform::State Platform::startup(
	pstd::FixedArena* arena,
	const char* windowName,
	const int windowWidth,
	const int windowHeight
) {
	pstd::Allocation stateAllocation{ pstd::fixedAlloc<StateImpl>(arena) };
	StateImpl* state{ new (stateAllocation.block) StateImpl{ .windowData{
		.isRunning = true,
		.eventBuffer =
			(KeyEvent*)pstd::fixedAlloc<KeyEvent>(arena, 1024).block } } };

	HINSTANCE hInstance{ GetModuleHandle(0) };

	const char windowClassName[]{ "window class" };

	WNDCLASS windowClass{ .lpfnWndProc = windowProc,
						  .hInstance = hInstance,
						  .lpszClassName = windowClassName };

	RegisterClass(&windowClass);

	bool windowIsFullscreen{ false };

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
		windowName,
		windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		adjustedWindowWidth,
		adjustedWindowHeight,
		0,
		0,
		hInstance,
		&state->windowData
	) };

	if (hwnd == 0) {
		return 0;
	}

	ShowWindow(hwnd, SW_SHOW);

	state->hwnd = hwnd;
	state->hInstance = hInstance;

	return stateAllocation.block;
}

void Platform::shutdown(Platform::State iState) {
	const auto state{ (StateImpl*)iState };
	DestroyWindow(state->hwnd);
}

bool Platform::isRunning(Platform::State iState) {
	const auto state{ (StateImpl*)iState };
	return state->windowData.isRunning;
}

pstd::FixedArray<KeyEvent> Platform::getKeyEventBuffer(Platform::State iState) {
	const auto state{ (StateImpl*)iState };
	return state->windowData.eventBuffer;
};

namespace {
	InputCode virtualToInputCode(const char vcode) {
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
		windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		LRESULT res{};

		WindowData* windowData{};
		if (uMsg == WM_CREATE) {
			CREATESTRUCT* createStruct{ reinterpret_cast<CREATESTRUCT*>(lParam
			) };
			windowData =
				reinterpret_cast<WindowData*>(createStruct->lpCreateParams);
			SetWindowLongPtr(
				hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowData)
			);
		} else {
			windowData = reinterpret_cast<WindowData*>(
				GetWindowLongPtr(hwnd, GWLP_USERDATA)
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
				InputCode iCode{ virtualToInputCode(wParam) };
				InputAction iAction{ InputAction::RELEASED };
				windowData->eventBuffer.pushBack({ .action = iAction,
												   .code = iCode });
			} break;
			case WM_KEYDOWN: {
				InputCode iCode{ virtualToInputCode(wParam) };
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
}  // namespace
