#pragma once
#include "Engine.h"
#include "STD/PHashMap.h"
#include "AssetLoader.h"

namespace Engine {
	// only the arrays corresponding to componentFlags will be allocated
	static constexpr size_t maxArchetypeCount{ 16 };

	struct Archetype {
		ComponentTypeFlags componentFlags;
		pstd::HashMap<UID, size_t> uidToIndex;

		pstd::Array<Transform> transforms;
		pstd::Array<MeshData> models;
	};

	Archetype createArchetype(
		pstd::Arena* pArena,
		Engine::ComponentTypeFlags componentFlags,
		size_t maxEntityCount
	);

	pstd::Array<Archetype*> getMatchingArchetypes(
		pstd::Arena* pArena,
		pstd::Array<Archetype>* pArchetypes,
		uint32_t componentFlags
	);

}  // namespace Engine
