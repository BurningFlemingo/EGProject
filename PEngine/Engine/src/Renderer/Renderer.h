#pragma once
#include "PTypes.h"
#include "PArena.h"
#include "PMemory.h"
#include "Platforms/Window.h"

namespace Renderer {
	struct State;

	size_t getSizeofState();

	State* startup(
		pstd::Arena* pPersistArena,
		pstd::ArenaPair scratchArenas,
		const Platform::State& platformState,
		pstd::AllocationRegistry* pAllocRegistry
	);

	void shutdown(State* state);
}  // namespace Renderer
