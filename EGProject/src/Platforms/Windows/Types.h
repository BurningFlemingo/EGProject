#pragma once
#include "PCircularBuffer.h"
#include "Events.h"
#include <Windows.h>

namespace {
	struct WindowData {
		bool isRunning;
		static constexpr size_t eventBufferCapacity{ 1024 };
		pstd::CircularBuffer<KeyEvent> eventBuffer;
	};

	struct InternalState {
		WindowData windowData;
		HWND hwnd;
		HINSTANCE hInstance;
	};
}  // namespace
