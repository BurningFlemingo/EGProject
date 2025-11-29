#pragma once
#include "STD/PTypes.h"
#include "STD/PArray.h"
#include "STD/PArena.h"
#include "STD/PCircularBuffer.h"

#include "Event.h"

namespace Platform {
	struct State;

	size_t getSizeofState();

	State* startup(
		pstd::Arena* pPersistArena,
		const char* windowName,
		const int windowWidth,
		const int windowHeight
	);

	bool popEvent(
		State* state, Event* eventOut
	);	// returns true if an event was popped

	bool isRunning(State* state);

	void update(State* state);

	void shutdown(State* state);
}  // namespace Platform
