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
		.uidToPath = pstd::createHashMap<UID, AssetPath>(pArena, maxAssets),
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
	AssetManager::registerMesh(State* pState, pstd::String path, UID uid) {
	AssetPath* pPaths{ pstd::find(&pState->uidToPath, uid) };
	if (!pPaths) {
		pState->uidToPath[uid] = {};
	} else {
		ASSERT(pPaths->meshPath.buffer != nullptr, "asset already registered");
	}

	pState->uidToPath[uid].meshPath = path;

	return uid;
}

AssetManager::UID
	AssetManager::registerTexture(State* pState, pstd::String path, UID uid) {
	AssetPath* pPaths{ pstd::find(&pState->uidToPath, uid) };
	if (!pPaths) {
		pState->uidToPath[uid] = {};
	} else {
		ASSERT(pPaths->meshPath.buffer != nullptr, "asset already registered");
	}

	pState->uidToPath[uid].texturePath = path;

	return uid;
}

void AssetManager::loadAsset(State* pState, UID uid) {
	size_t* pMeshIndex{ pstd::find(&pState->uidToLoadedMeshIndex, uid) };
	size_t* pTextureIndex{ pstd::find(&pState->uidToLoadedTextureIndex, uid) };

	if (pMeshIndex == nullptr) {
		size_t meshIndex{ pState->loadedMeshes.count };
		Engine::MeshData mesh{
			Engine::loadMesh(&pState->arena, pState->uidToPath[uid].meshPath)
		};
		pstd::pushBack(&pState->loadedMeshes, mesh);

		pState->uidToLoadedMeshIndex[uid] = meshIndex;
	}

	// TODO: create asset type bitflags
	// if (pTextureIndex == nullptr) {
	// 	size_t textureIndex{ pState->loadedTextures.count };
	// 	Engine::TextureData texture{
	// 		Engine::loadTexture(&pState->arena, pState->uidToPath[uid])
	// 	};
	// 	pstd::pushBack(&pState->loadedTextures, texture);

	// 	pState->uidToLoadedMeshIndex[uid] = textureIndex;
	// }
}

Engine::MeshData* AssetManager::retrieveMesh(State* pState, UID uid) {
	size_t index{};
	bool isLoaded{ pstd::find(&pState->uidToLoadedMeshIndex, uid, &index) };

	if (!isLoaded) {
		Engine::MeshData mesh{
			Engine::loadMesh(&pState->arena, pState->uidToPath[uid].meshPath)
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
		Engine::TextureData texture{ Engine::loadTexture(
			&pState->arena, pState->uidToPath[uid].texturePath
		) };

		index = pState->loadedTextures.count;
		pstd::pushBack(&pState->loadedTextures, texture);

		pState->uidToLoadedTextureIndex[uid] = index;
	}

	return &pState->loadedTextures[index];
}
