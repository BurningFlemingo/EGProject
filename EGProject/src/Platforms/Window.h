#pragma once
#include "PTypes.h"
#include "PArray.h"
#include "PArena.h"
#include "PCircularBuffer.h"

#include "Events.h"

namespace Platform {
	using State = void*;

	size_t getSizeofState();

	State startup(
		pstd::FixedArena* stateArena,
		const char* windowName,
		const int windowWidth,
		const int windowHeight
	);

	pstd::FixedArray<KeyEvent> popKeyEvents(State state);
	bool isRunning(State state);

	void update(State state);

	void shutdown(State state);
}  // namespace Platform
