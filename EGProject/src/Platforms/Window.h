#pragma once
#include <stdint.h>
#include "PArray.h"
#include "PArena.h"
#include "Events.h"

namespace Platform {
	using State = void*;

	size_t getPlatformAllocSize();

	State startup(
		pstd::FixedArena* arena,
		const char* windowName,
		const int windowWidth,
		const int windowHeight
	);

	pstd::FixedArray<KeyEvent> getKeyEventBuffer(State iState);
	bool isRunning(State iState);

	void shutdown(State state);
}  // namespace Platform
