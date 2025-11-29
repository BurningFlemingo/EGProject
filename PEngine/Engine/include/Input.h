#pragma once
#include "STD/PTypes.h"

namespace Engine {
	enum class InputAction : uint32_t {
		INVALID = 0,
		PRESSED = 1,
		RELEASED = 2,

		COUNT
	};

	struct KeyState {
		bool wasPressed;
		bool wasReleased;
		bool isDown;
		bool isUp;
	};

	// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	enum class KeyCode : uint32_t {
		INVALID = 0,
		BACKSPACE = 8,
		TAB = 9,
		ENTER = 13,
		ESC = 27,
		SPACE = 32,

		ZERO = '0',
		ONE = '1',
		TWO = '2',
		THREE = '3',
		FOUR = '4',
		FIVE = '5',
		SIX = '6',
		SEVEN = '7',
		EIGHT = '8',
		NINE = '9',

		A = 'A',
		B = 'B',
		C = 'C',
		D = 'D',
		E = 'E',
		F = 'F',
		G = 'G',
		H = 'H',
		I = 'I',
		J = 'J',
		K = 'K',
		L = 'L',
		M = 'M',
		N = 'N',
		O = 'O',
		P = 'P',
		Q = 'Q',
		R = 'R',
		S = 'S',
		T = 'T',
		U = 'U',
		V = 'V',
		W = 'W',
		X = 'X',
		Y = 'Y',
		Z = 'Z',

		SHIFT = 128,
		CTRL,
		ALT,

		UP,
		DOWN,
		LEFT,
		RIGHT,

		COUNT
	};
}  // namespace Engine
