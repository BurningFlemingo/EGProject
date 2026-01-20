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

	void setTransforms(
		Renderer::State* pState, pstd::Span<Engine::Transform> transforms
	);

	void setModels(
		State* pState,
		AssetManager::State* pAssetManager,
		pstd::Arena scratchArena,
		pstd::Span<AssetManager::UID> meshIDs
	);

	void render(
		State* state, const Platform::State& platformState, bool windowResized
	);
	void shutdown(State* state);
}  // namespace Renderer
