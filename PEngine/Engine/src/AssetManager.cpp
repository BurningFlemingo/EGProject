#include "AssetManager.h"
#include "AssetLoader.h"
#include "STD/PHashMap.h"
#include "STD/PArray.h"
#include <new>

namespace {}  // namespace

AssetManager::State* AssetManager::startup(
	pstd::Arena* pArena, size_t maxAssets, size_t bytesAllocated
) {
	return new (pstd::alloc<State>(pArena)) State{
		.arena = pstd::createArena(pArena, bytesAllocated),
		.uidToPath = pstd::createHashMap<UID, pstd::String>(pArena, maxAssets),
		.uidToLoadedMeshIndex =
			pstd::createHashMap<UID, size_t>(pArena, maxAssets),
		.uidToLoadedTextureIndex =
			pstd::createHashMap<UID, size_t>(pArena, maxAssets),
		.loadedMeshes =
			pstd::createArray<Engine::MeshData>(pArena, maxAssets, 0),
		.loadedTextures =
			pstd::createArray<Engine::TextureData>(pArena, maxAssets, 0),
	};
}

AssetManager::UID
	AssetManager::registerAsset(State* pState, pstd::String path, UID uid) {
	ASSERT(!pstd::contains(pState->uidToPath, uid), "asset already registered");

	pState->uidToPath[uid] = path;

	return uid;
}

Engine::MeshData* AssetManager::retrieveMesh(State* pState, UID uid) {
	size_t index{};
	bool isLoaded{ pstd::find(&pState->uidToLoadedMeshIndex, uid, &index) };

	if (!isLoaded) {
		ASSERT(pstd::contains(pState->uidToPath, uid), "asset not registered");

		Engine::MeshData mesh{
			Engine::loadMesh(&pState->arena, pState->uidToPath[uid])
		};

		index = pState->loadedMeshes.count;
		pstd::pushBack(&pState->loadedMeshes, mesh);

		pState->uidToLoadedMeshIndex[uid] = index;
	}

	return &pState->loadedMeshes[index];
}

Engine::TextureData* AssetManager::retrieveTexture(State* pState, UID uid) {
	size_t index{};
	bool isLoaded{ pstd::find(&pState->uidToLoadedTextureIndex, uid, &index) };

	if (!isLoaded) {
		ASSERT(pstd::contains(pState->uidToPath, uid), "asset not registered");

		Engine::TextureData texture{
			Engine::loadTexture(&pState->arena, pState->uidToPath[uid])
		};

		index = pState->loadedTextures.count;
		pstd::pushBack(&pState->loadedTextures, texture);

		pState->uidToLoadedTextureIndex[uid] = index;
	}

	return &pState->loadedTextures[index];
}
