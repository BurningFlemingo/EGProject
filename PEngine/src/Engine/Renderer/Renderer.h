#pragma once
#include "Core/PTypes.h"
#include "Core/PArena.h"
#include "Core/PMemory.h"
#include "Engine/Platforms/Window.h"

namespace Renderer {
	struct State;

	size_t getSizeofState();

	State* startup(
		pstd::Arena* pPersistArena,
		pstd::Arena scratchArena,
		const Platform::State& platformState
	);

	void shutdown(State* state);
}  // namespace Renderer
