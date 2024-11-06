#pragma once
#include "PTypes.h"
#include "PArena.h"

namespace Renderer {
	struct State;

	size_t getSizeofState();

	State* startup(pstd::FixedArena* stateArena, pstd::FixedArena scratchArena);
	void shutdown(State* state);
}  // namespace Renderer
