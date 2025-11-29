#pragma once

#include "Input.h"
#include "Platforms/Window.h"

#include "STD/PCircularBuffer.h"

#include <Windows.h>

struct WindowData {
	bool isRunning;
	static constexpr size_t eventBufferCapacity{ 1024 };
	pstd::CircularBuffer<Platform::Event> eventBuffer;

	int xMousePosition;
	int yMousePosition;
};

namespace Platform {
	struct State {
		WindowData windowData;
		HWND hwnd;
		HINSTANCE hInstance;
	};
}  // namespace Platform
