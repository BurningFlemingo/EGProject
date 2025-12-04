#pragma once
#include "STD/PArena.h"
#include "STD/PString.h"

namespace AssetManager {
	using UID = size_t;

	struct State;

	State*
		startup(pstd::Arena* pArena, size_t maxAssets, size_t bytesAllocated);
	UID getUID(State* pState, pstd::String assetName);
	UID load(State* pState, pstd::String name);
}  // namespace AssetManager
