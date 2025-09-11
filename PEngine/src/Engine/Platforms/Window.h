#pragma once
#include "Core/PTypes.h"
#include "Core/PArray.h"
#include "Core/PArena.h"
#include "Core/PCircularBuffer.h"

#include "Engine/Events.h"

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
