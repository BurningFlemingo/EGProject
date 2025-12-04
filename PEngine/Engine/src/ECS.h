#pragma once
#include "Engine.h"
#include "STD/PHashMap.h"
#include "AssetLoader.h"
#include "STD/PString.h"
#include "AssetManager.h"

namespace Engine {
	static constexpr size_t maxArchetypeCount{ 16 };

	// only the arrays corresponding to componentFlags will be allocated
	struct Archetype {
		ComponentTypeFlags componentFlags;
		pstd::HashMap<UID, size_t> uidToIndex;

		pstd::Array<Transform> transforms;
		pstd::Array<AssetManager::UID> assetIDs;
	};

	Archetype createArchetype(
		pstd::Arena* pArena,
		Engine::ComponentTypeFlags componentFlags,
		size_t maxEntityCount
	);

	pstd::String stringify(pstd::Arena* pArena, Engine::Entity entity);
	pstd::String stringify(pstd::Arena* pArena, Engine::Archetype archetype);

}  // namespace Engine
