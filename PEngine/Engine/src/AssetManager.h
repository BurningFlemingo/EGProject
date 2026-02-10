#pragma once

#include "STD/PCircularBuffer.h"
#include "STD/PString.h"
#include "STD/PHashMap.h"
#include "STD/PArray.h"
#include "AssetLoader.h"
#include "Assets.h"

namespace AssetManager {
	using UID = size_t;

	struct AssetPath {
		pstd::String meshPath;
		pstd::String texturePath;

		bool operator=(const AssetPath& other) {
			bool matches{ pstd::stringsMatch(this->meshPath, other.meshPath) };
			matches &= pstd::stringsMatch(this->texturePath, other.texturePath);

			return matches;
		}
	};

	struct State {
		pstd::Arena arena;

		pstd::HashMap<UID, AssetPath> uidToPath;

		pstd::HashMap<UID, size_t> uidToLoadedMeshIndex;
		pstd::HashMap<UID, size_t> uidToLoadedTextureIndex;

		pstd::Array<Engine::MeshData> loadedMeshes;
		pstd::Array<Engine::TextureData> loadedTextures;
	};

	State*
		startup(pstd::Arena* pArena, size_t maxAssets, size_t bytesAllocated);

	Engine::MeshData* retrieveMesh(State* pState, UID uid);
	Engine::TextureData* retrieveTexture(State* pState, UID uid);
}  // namespace AssetManager
