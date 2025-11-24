#pragma once
#include "STD/PTypes.h"
#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "STD/PMatrix.h"
#include "Platforms/Window.h"
#include "AssetLoader.h"
#include "EngineState.h"

namespace Engine {
	using UID = size_t;
}

namespace Renderer {
	struct State;

	size_t getSizeofState();

	State* startup(
		pstd::AllocationRegistry* pAllocRegistry,
		pstd::Arena* pPersistArena,
		pstd::Arena scratchArena,
		const Platform::State& platformState
	);

	void setupFrame(
		State* pState,
		pstd::Arena scratchArena,
		pstd::Span<pstd::MeshData> mesh,
		pstd::Span<Engine::Transform> transforms
	);
	void render(State* state, bool windowResized);
	void shutdown(State* state);
}  // namespace Renderer
