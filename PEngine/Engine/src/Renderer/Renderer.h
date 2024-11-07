#pragma once
#include "PTypes.h"
#include "PArena.h"
#include "Platforms/Window.h"

namespace Renderer {
	struct State;

	size_t getSizeofState();

	State* startup(
		pstd::FixedArena* stateArena,
		pstd::FixedArena scratchArena,
		const Platform::State& platformState
	);

	void shutdown(State* state);
}  // namespace Renderer
