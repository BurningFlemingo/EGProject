#pragma once
#include "Engine.h"
#include "STD/PHashMap.h"
#include "AssetLoader.h"
#include "STD/PString.h"

namespace Engine {
	// only the arrays corresponding to componentFlags will be allocated
	static constexpr size_t maxArchetypeCount{ 16 };

	struct Archetype {
		ComponentTypeFlags componentFlags;
		pstd::HashMap<UID, size_t> uidToIndex;

		pstd::Array<Transform> transforms;
		pstd::Array<Model> models;
	};

	Archetype createArchetype(
		pstd::Arena* pArena,
		Engine::ComponentTypeFlags componentFlags,
		size_t maxEntityCount
	);

	pstd::String stringify(pstd::Arena* pArena, Engine::Entity entity);
	pstd::String stringify(pstd::Arena* pArena, Engine::Archetype archetype);

}  // namespace Engine
