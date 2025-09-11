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
		pstd::ArenaPair scratchArenas,
		const Platform::State& platformState,
		pstd::AllocationRegistry* pAllocRegistry
	);

	void shutdown(State* state);
}  // namespace Renderer
