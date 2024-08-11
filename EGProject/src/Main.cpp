#include <Windows.h>
#include <iostream>

struct WindowData {
	bool running;
};

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
			windowData->running = false;
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

	WindowData windowData{ .running = true };

	HWND hwnd{ CreateWindowEx(
		0,
		windowClassName,
		"App Name",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		hInstance,
		&windowData
	) };

	if (hwnd == 0) {
		return 0;
	}

	ShowWindow(hwnd, 1);

	while (windowData.running) {
		MSG msg{};
		while (GetMessage(&msg, 0, 0, 0) != 0 && windowData.running) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}
