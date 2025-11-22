#pragma once
#include "STD/PTypes.h"
#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "STD/PMatrix.h"
#include "Platforms/Window.h"
#include "AssetLoader.h"

namespace Renderer {
	struct State;

	size_t getSizeofState();

	State* startup(
		pstd::Arena* pPersistArena,
		pstd::Arena scratchArena,
		const Platform::State& platformState,
		pstd::OBJ obj
	);

	void render(State* state, bool windowResized);
	void shutdown(State* state);
}  // namespace Renderer
