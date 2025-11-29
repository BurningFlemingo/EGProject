#pragma once
#include "STD/PTypes.h"
#include "STD/PArray.h"
#include "STD/PArena.h"
#include "STD/PCircularBuffer.h"

#include "Input.h"

namespace Platform {
	struct State;

	enum class EventType { key, window, COUNT };

	struct Event {
		EventType type;
		union {
			struct {
				InputAction action;
				InputCode virtualCode;
				InputCode physicalCode;
			} keyEvent;
			struct {
				bool resized;
			} windowEvent;
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

	void captureCursor(State* pState);
	void releaseCursor(State* pState);

	void hideCursor(State* pState);
	void showCursor(State* pState);

	bool isRunning(State* state);

	void update(State* state);

	void shutdown(State* state);
}  // namespace Platform
