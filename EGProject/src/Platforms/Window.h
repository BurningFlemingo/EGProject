#pragma once
#include <stdint.h>
#include "PArray.h"
#include "PArena.h"
#include "PCircularBuffer.h"

#include "Events.h"

namespace Platform {
	using State = void*;

	size_t getSubsystemAllocSize();

	State startup(
		pstd::FixedArena* arena,
		const char* windowName,
		const int windowWidth,
		const int windowHeight
	);

	pstd::FixedArray<KeyEvent> popKeyEvents(State iState);
	bool isRunning(State iState);

	void shutdown(State state);
}  // namespace Platform
