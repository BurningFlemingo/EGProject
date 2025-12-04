#include "AssetManager.h"
#include "AssetLoader.h"
#include "STD/PHashMap.h"
#include <new>

AssetManager::State* AssetManager::startup(
	pstd::Arena* pArena, size_t maxAssets, size_t bytesAllocated
) {
	return new (pstd::alloc<State>(pArena)) State{
		.arena = pstd::createArena(pArena, bytesAllocated),
		.nameToUID = pstd::createHashMap<pstd::String, UID>(pArena, maxAssets),
		.uidToPath = pstd::createHashMap<UID, pstd::String>(pArena, maxAssets),
		.uidToLoadedMeshIndex =
			pstd::createHashMap<UID, size_t>(pArena, maxAssets),
		.loadedMeshes =
			pstd::createArray<Engine::MeshData>(pArena, maxAssets, 0),
	};
}

AssetManager::UID AssetManager::getUID(State* pState, pstd::String assetName) {
	static UID nextFreeUID{};

	UID* pUID{ pstd::find(&pState->nameToUID, assetName) };
	UID uid{};
	if (pUID == nullptr) {
		uid = nextFreeUID;
		pState->nameToUID[assetName] = uid;
		pState->uidToPath[uid] = assetName;	 // TODO: change this to use some
											 // register name -> path thing
		nextFreeUID++;
	} else {
		uid = *pUID;
	}
	return uid;
}

AssetManager::UID AssetManager::load(State* pState, pstd::String assetName) {
	UID uid{ getUID(pState, assetName) };
	size_t* pIndex{ pstd::find(&pState->uidToLoadedMeshIndex, uid) };
	if (pIndex == nullptr) {
		size_t meshIndex{ pState->loadedMeshes.count };
		Engine::MeshData mesh{
			Engine::loadMesh(&pState->arena, pState->uidToPath[uid])
		};
		pstd::pushBack(&pState->loadedMeshes, mesh);

		pState->uidToLoadedMeshIndex[uid] = meshIndex;
	}
	return uid;
}
Engine::MeshData* AssetManager::retrieveMesh(State* pState, UID uid) {
	size_t index{ pState->uidToLoadedMeshIndex[uid] };
	return &pState->loadedMeshes[index];
}
