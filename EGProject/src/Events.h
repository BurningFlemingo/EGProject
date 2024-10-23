#pragma once
#include <stdint.h>

enum class InputAction : uint32_t {
	INVALID = 0,
	PRESSED = 1,
	RELEASED = 2,
	REPEATING = 3,

	COUNT
};

enum class InputCode : uint32_t {
	INVALID = 0,
	BACKSPACE = 8,
	TAB = 9,
	ENTER = 27,
	ESC = 27,
	SPACE = 32,

	SHIFT = 128,
	CTRL,
	ALT,

	LEFT_MB,
	RIGHT_MB,
	MIDDLE_MB,

	UP,
	DOWN,
	LEFT,
	RIGHT,

	COUNT
};

struct KeyEvent {
	InputAction action;
	InputCode code;
};
