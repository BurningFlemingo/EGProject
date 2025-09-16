#pragma once

#include "Events.h"
#include "Platforms/Window.h"

#include "Core/PCircularBuffer.h"

#include <Windows.h>

struct WindowData {
	bool isRunning;
	static constexpr size_t eventBufferCapacity{ 1024 };
	pstd::CircularBuffer<Platform::Event> eventBuffer;
};

namespace Platform {
	struct State {
		WindowData windowData;
		HWND hwnd;
		HINSTANCE hInstance;
	};
}  // namespace Platform
