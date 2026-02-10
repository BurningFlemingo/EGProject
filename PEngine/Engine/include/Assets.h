#pragma once
#include "STD/PArena.h"
#include "STD/PString.h"

namespace AssetManager {
	using UID = size_t;

	struct State;

	UID getUID(State* pState, pstd::String assetName);
	void loadAsset(State* pState, UID uid);

	UID registerTexture(State* pState, pstd::String path, UID uid);
	UID registerMesh(State* pState, pstd::String path, UID uid);
}  // namespace AssetManager
