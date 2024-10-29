#pragma once
#include "PTypes.h"
#include "PArena.h"

namespace Renderer {
	using State = void*;

	size_t getSizeofState();

	State startup(pstd::FixedArena* stateArena, pstd::FixedArena scratchArena);
	void shutdown(State state);
}  // namespace Renderer
