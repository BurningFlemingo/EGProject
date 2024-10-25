#include "Platforms/Window.h"

#include "PCircularBuffer.h"
#include "PMemory.h"
#include "Events.h"

#include "Base.h"
#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "PArray.h"
#include "PAlgorithm.h"
#include <new>

namespace {
	struct WindowData {
		bool isRunning;
		static constexpr size_t eventBufferCapacity{ 1024 };
		pstd::CircularBuffer<KeyEvent> eventBuffer{};
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

size_t Platform::getSubsystemAllocSize() {
	size_t stateTypeSize{ sizeof(StateImpl) };
	size_t bufferSize{ sizeof(KeyEvent) * WindowData::eventBufferCapacity };
	// TODO: this is a temp fix, find padding required for state type through a
	// function
	size_t padding{ 4 };
	size_t totalSize{ stateTypeSize + bufferSize + padding };
	return totalSize;
}

Platform::State Platform::startup(
	pstd::FixedArena* arena,
	const char* windowName,
	const int windowWidth,
	const int windowHeight
) {
	pstd::Allocation stateAllocation{ pstd::bufferAlloc<StateImpl>(arena) };
	pstd::Allocation eventBufferAllocation{
		pstd::bufferAlloc<KeyEvent>(arena, WindowData::eventBufferCapacity)
	};

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

	// the window data pointer passed here cant be local since it will be
	// refrenced after this function in windowProc
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
		&((StateImpl*)stateAllocation.block)->windowData
	) };

	if (hwnd == 0) {
		return 0;
	}

	ShowWindow(hwnd, SW_SHOW);

	WindowData windowData{ .isRunning = true,
						   .eventBuffer = { .allocation =
												eventBufferAllocation } };

	new (stateAllocation.block) StateImpl{ .windowData = windowData,
										   .hwnd = hwnd,
										   .hInstance = hInstance };

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

pstd::FixedArray<KeyEvent> Platform::popKeyEvents(Platform::State iState) {
	const auto state{ (StateImpl*)iState };
	pstd::FixedArray<KeyEvent> eventArray{
		pstd::getContents(state->windowData.eventBuffer)
	};
	state->windowData.eventBuffer.tailIndex =
		state->windowData.eventBuffer.headIndex;

	return eventArray;
}

namespace {
	InputCode virtualToInputCode(const char vcode) {
		if ((vcode >= 'A' && vcode <= 'Z') || (vcode >= '0' && vcode <= '9')) {
			return (InputCode)vcode;
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
				KeyEvent event{ .action = iAction, .code = iCode };
				pstd::pushBack(&windowData->eventBuffer, event);
			} break;
			case WM_KEYDOWN: {
				InputCode iCode{ virtualToInputCode(wParam) };
				InputAction iAction{ InputAction::PRESSED };
				KeyEvent event{ .action = iAction, .code = iCode };
				pstd::pushBack(&windowData->eventBuffer, event);
			} break;
			default: {
				res = DefWindowProc(hwnd, uMsg, wParam, lParam);
			}
		}

		return res;
	}
}  // namespace
