#include "Platforms/Window.h"
#include "Cursor.h"

#include "Input.h"
#include "Platforms/Event.h"
#include "STD/PCircularBuffer.h"
#include "STD/PMemory.h"
#include "STD/PAlgorithm.h"
#include "STD/PArray.h"

#include "Base.h"
#include "Platforms/Windows/Types.h"

#include <Windows.h>
#include <winuser.h>
#include <Windowsx.h>
#include <hidusage.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <new>
#include "Logging.h"

using Engine::KeyCode;
using Engine::InputAction;

namespace {
	LRESULT CALLBACK
		windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	KeyCode virtualToKeyCode(const uint8_t vcode);
	KeyCode physicalToKeyCode(const uint8_t scancode);
}  // namespace

Platform::State* Platform::startup(
	pstd::Arena* pPersistArena,
	const char* windowName,
	const int windowWidth,
	const int windowHeight
) {
	State* state{ pstd::alloc<State>(pPersistArena) };
	Event* eventBufferBlock{
		pstd::alloc<Event>(pPersistArena, WindowData::eventBufferCapacity)
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

	WindowData windowData{ .isRunning = true,
						   .eventBuffer = {
							   .block = eventBufferBlock,
							   .size = WindowData::eventBufferCapacity } };
	state->windowData = windowData;

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
		&state->windowData
	) };

	ASSERT(hwnd != NULL);

	ShowWindow(hwnd, SW_SHOW);

	RAWINPUTDEVICE rawInputDevices[1] = { {
		.usUsagePage = HID_USAGE_PAGE_GENERIC,
		.usUsage = HID_USAGE_GENERIC_MOUSE,
		.hwndTarget = hwnd,
	} };

	RegisterRawInputDevices(rawInputDevices, 1, sizeof(rawInputDevices[0]));

	return new (state)
		State{ .windowData = windowData, .hwnd = hwnd, .hInstance = hInstance };
}

void Platform::shutdown(Platform::State* state) {
	DestroyWindow(state->hwnd);
}

void Platform::update(State* state) {
	MSG msg{};
	bool windowRunning{ state->windowData.isRunning };
	while (PeekMessageA(&msg, 0, 0, 0, true) != 0 && windowRunning) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool Platform::isRunning(Platform::State* state) {
	return state->windowData.isRunning;
}

bool Platform::popEvent(Platform::State* state, Event* outEvent) {
	return pstd::popBack(&state->windowData.eventBuffer, outEvent);
}

void Platform::captureCursor(State* pState) {
	RECT rect;
	GetClientRect(pState->hwnd, &rect);

	ClientToScreen(pState->hwnd, rcast<POINT*>(&rect.left));
	ClientToScreen(pState->hwnd, rcast<POINT*>(&rect.right));

	ClipCursor(&rect);
}
void Platform::releaseCursor(State* pState) {
	ClipCursor(NULL);
}

void Platform::hideCursor(State* pState) {
	ShowCursor(FALSE);
}
void Platform::showCursor(State* pState) {
	ShowCursor(TRUE);
}

namespace {
	Engine::KeyCode virtualToKeyCode(const uint8_t vcode) {
		if ((vcode >= 'A' && vcode <= 'Z') || (vcode >= '0' && vcode <= '9')) {
			return (KeyCode)vcode;
		}

		Engine::KeyCode keyCode{};
		switch (vcode) {
		case VK_BACK:
			keyCode = KeyCode::BACKSPACE;
			break;
		case VK_TAB:
			keyCode = KeyCode::TAB;
			break;
		case 0x0D:
			keyCode = KeyCode::ENTER;
			break;
		case VK_ESCAPE:
			keyCode = KeyCode::ESC;
			break;
		case VK_SPACE:
			keyCode = KeyCode::SPACE;
			break;
		case VK_SHIFT:
			keyCode = KeyCode::SHIFT;
			break;
		case VK_CONTROL:
			keyCode = KeyCode::CTRL;
			break;
		case VK_MENU:
			keyCode = KeyCode::ALT;
			break;
		case VK_UP:
			keyCode = KeyCode::UP;
			break;
		case VK_DOWN:
			keyCode = KeyCode::DOWN;
			break;
		case VK_LEFT:
			keyCode = KeyCode::LEFT;
			break;
		case VK_RIGHT:
			keyCode = KeyCode::RIGHT;
			break;
		default:
			keyCode = KeyCode::INVALID;
			break;
		}
		return keyCode;
	}

	KeyCode physicalToKeyCode(const uint8_t scancode) {
		KeyCode keyCode{ KeyCode::INVALID };
		switch (scancode) {
		case 0x001E:
			keyCode = KeyCode::A;
			break;
		case 0x0030:
			keyCode = KeyCode::B;
			break;
		case 0x002E:
			keyCode = KeyCode::C;
			break;
		case 0x0020:
			keyCode = KeyCode::D;
			break;
		case 0x0012:
			keyCode = KeyCode::E;
			break;
		case 0x0021:
			keyCode = KeyCode::F;
			break;
		case 0x0022:
			keyCode = KeyCode::G;
			break;
		case 0x0023:
			keyCode = KeyCode::H;
			break;
		case 0x0017:
			keyCode = KeyCode::I;
			break;
		case 0x0024:
			keyCode = KeyCode::J;
			break;
		case 0x0025:
			keyCode = KeyCode::K;
			break;
		case 0x0026:
			keyCode = KeyCode::L;
			break;
		case 0x0032:
			keyCode = KeyCode::M;
			break;
		case 0x0031:
			keyCode = KeyCode::N;
			break;
		case 0x0018:
			keyCode = KeyCode::O;
			break;
		case 0x0019:
			keyCode = KeyCode::P;
			break;
		case 0x0010:
			keyCode = KeyCode::Q;
			break;
		case 0x0013:
			keyCode = KeyCode::R;
			break;
		case 0x001F:
			keyCode = KeyCode::S;
			break;
		case 0x0014:
			keyCode = KeyCode::T;
			break;
		case 0x0016:
			keyCode = KeyCode::U;
			break;
		case 0x002F:
			keyCode = KeyCode::V;
			break;
		case 0x0011:
			keyCode = KeyCode::W;
			break;
		case 0x002D:
			keyCode = KeyCode::X;
			break;
		case 0x0015:
			keyCode = KeyCode::Y;
			break;
		case 0x002C:
			keyCode = KeyCode::Z;
			break;
		}

		return keyCode;
	}

	struct Dimensions {
		int width;
		int height;
	};

	Dimensions calcClientDimensions(HWND hwnd) {
		RECT clientRect{};
		GetClientRect(hwnd, &clientRect);
		int width{ clientRect.right - clientRect.left };
		int height{ clientRect.bottom - clientRect.top };

		return { .width = width, .height = height };
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
		} break;
		case WM_DESTROY: {
			windowData->isRunning = false;
		} break;
			// https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#virtual-key-codes-described
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			auto scancode{ ncast<uint8_t>((lParam >> 16) & 0xFF) };
			auto keyWasDown{ ncast<bool>((lParam >> 30) & 0x1) };
			auto keyIsUp{ ncast<bool>((lParam >> 31) & 0x1) };

			if (keyWasDown && !keyIsUp) {
				// not handling repeating, repeate event can come after last
				// keyup, causing issues like sticky keys;
				break;
			}

			KeyCode virtualCode{ virtualToKeyCode(wParam) };
			KeyCode physicalCode{ physicalToKeyCode(scancode) };

			Platform::CompressedKeyState keyState{ .isDown = !keyIsUp,
												   .wasUp = !keyWasDown };

			Platform::Event event{ .type = Platform::EventType::key,
								   .keyEvent = { .state = keyState,
												 .virtualCode = virtualCode,
												 .physicalCode =
													 physicalCode } };

			pstd::pushBack(&windowData->eventBuffer, event);
		} break;
		case WM_INPUT: {
			UINT dwSize;
			GetRawInputData(
				(HRAWINPUT)lParam,
				RID_INPUT,
				NULL,
				&dwSize,
				sizeof(RAWINPUTHEADER)
			);
			constexpr UINT headerSize{ 48 };
			ASSERT(dwSize <= headerSize);

			LPBYTE lpb[headerSize]{};
			GetRawInputData(
				(HRAWINPUT)lParam,
				RID_INPUT,
				lpb,
				&dwSize,
				sizeof(RAWINPUTHEADER)
			);

			RAWINPUT* raw{ (RAWINPUT*)lpb };

			if (raw->header.dwType == RIM_TYPEMOUSE) {
				float relXPos{ ncast<float>(raw->data.mouse.lLastX) };
				float relYPos{ -ncast<float>(raw->data.mouse.lLastY
				) };  // flipped because +Y is up in engine, while here -Y is up

				Platform::Event event{ .type = Platform::EventType::cursor,
									   .cursorEvent = { .dx = relXPos,
														.dy = relYPos } };
				pstd::pushBack(&windowData->eventBuffer, event);
			}
		} break;
		case WM_KILLFOCUS: {
			Platform::Event event{ .type = Platform::EventType::window,
								   .windowEvent = { .lostFocus = true } };
			pstd::pushBack(&windowData->eventBuffer, event);
		} break;
		case WM_SIZE: {
			Platform::Event event{ .type = Platform::EventType::window,
								   .windowEvent = { .resized = true } };
			pstd::pushBack(&windowData->eventBuffer, event);
		} break;
		default: {
			res = DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		}

		return res;
	}
}  // namespace
