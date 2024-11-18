#pragma once
#include "PTypes.h"
#include "PArena.h"
#include "Platforms/Window.h"

namespace Renderer {
	struct State;

	size_t getSizeofState();

	State* startup(
		pstd::ArenaFrame&& arenaFrame, const Platform::State& platformState
	);

	void shutdown(State* state);
}  // namespace Renderer
