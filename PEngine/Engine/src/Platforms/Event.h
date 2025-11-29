#pragma once

#include "Input.h"

namespace Platform {
	enum class EventType { key, cursor, window, COUNT };

	struct CompressedKeyState {
		bool isDown;
		bool wasUp;
	};

	struct Event {
		EventType type;
		union {
			struct {
				CompressedKeyState state;
				Engine::KeyCode virtualCode;
				Engine::KeyCode physicalCode;
			} keyEvent;
			struct {
				float dx;
				float dy;
			} cursorEvent;
			struct {
				bool resized;
				bool lostFocus;
			} windowEvent;
		};
	};

}  // namespace Platform
