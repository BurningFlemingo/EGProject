#include <Windows.h>

LRESULT CALLBACK
	WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT res{};
	res = DefWindowProc(hwnd, uMsg, wParam, lParam);
	return res;
}

int main() {
	HINSTANCE hInstance{ GetModuleHandle(0) };

	const wchar_t windowClassName[]{ L"window class" };

	WNDCLASSW windowClass{ .lpfnWndProc = WindowProc,
						   .hInstance = hInstance,
						   .lpszClassName = windowClassName };

	RegisterClassW(&windowClass);

	HWND hwnd{ CreateWindowExW(
		0,
		windowClassName,
		L"App Name",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		hInstance,
		0
	) };

	if (hwnd == 0) {
		return 0;
	}

	ShowWindow(hwnd, 1);

	while (true) {}
}
