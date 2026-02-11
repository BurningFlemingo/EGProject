#pragma once
#include "STD/PArena.h"
#include "STD/PString.h"

namespace AssetManager {
	using UID = size_t;

	struct State;

	UID getUID(State* pState, pstd::String assetName);

	UID registerAsset(State* pState, pstd::String path, UID uid);
}  // namespace AssetManager
