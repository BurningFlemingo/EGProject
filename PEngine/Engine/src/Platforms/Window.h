#pragma once
#include "PTypes.h"
#include "PArray.h"
#include "PArena.h"
#include "PCircularBuffer.h"

#include "Events.h"

namespace Platform {
	struct State;

	enum class EventType { key, COUNT };

	struct Event {
		EventType type;
		union {
			struct {
				InputAction action;
				InputCode code;
			} keyEvent;
		};
	};

	size_t getSizeofState();

	State* startup(
		pstd::FixedArenaFrame&& arenaFrame,
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
