#pragma once
#include "STD/PTypes.h"

enum class InputAction : uint32_t {
	INVALID = 0,
	PRESSED = 1,
	RELEASED = 2,
	REPEATING = 3,

	COUNT
};

// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
enum class InputCode : uint32_t {
	INVALID = 0,
	BACKSPACE = 8,
	TAB = 9,
	ENTER = 13,
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
