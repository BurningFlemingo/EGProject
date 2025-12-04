#include "ECS.h"
#include "AssetManager.h"
#include "EngineState.h"
#include "Engine.h"

Engine::Entity Engine::getEntity(Engine::State* pEngine, pstd::String name) {
	return pEngine->nameToEntity[name];
}

Engine::Entity Engine::createEntity(
	Engine::State* pEngine, pstd::String name, ComponentTypeFlags componentFlags
) {
	Entity entity{ createEntity(pEngine, componentFlags) };

	ASSERT(
		!pstd::exists(pEngine->nameToEntity, name), "component already created"
	);

	pEngine->nameToEntity[name] = entity;

	return entity;
}

Engine::Entity Engine::createEntity(
	Engine::State* pEngine, ComponentTypeFlags componentFlags
) {
	static size_t uid{};
	uid++;

	Archetype* pArchetype{};
	size_t archetypeIndex{ ncast<size_t>(-1) };
	bool found{};
	for (size_t i{}; i < pEngine->archetypes.count; i++) {
		pArchetype = &pEngine->archetypes[i];
		if (pArchetype->componentFlags == componentFlags) {
			archetypeIndex = i;
			found = true;
			break;
		}
	}
	if (!found) {
		archetypeIndex = pEngine->archetypes.count;

		Archetype archetype{ createArchetype(
			&pEngine->subsystemArena, componentFlags, Engine::maxEntityCount
		) };

		pstd::pushBack(&pEngine->archetypes, archetype);
		pArchetype = &pEngine->archetypes[archetypeIndex];
	}

	if (componentFlags & TransformComponent) {
		pstd::pushBack(&pArchetype->transforms, {});
	}
	if (componentFlags & AssetComponent) {
		pstd::pushBack(&pArchetype->assetIDs, {});
	}

	pArchetype->uidToIndex[uid] = pArchetype->uidToIndex.count;
	Entity entity{
		.uid = uid,
		.typeFlags = componentFlags,
		.archetypeIndex = archetypeIndex,
	};

	return entity;
}

template<>
Engine::Transform* Engine::getComponent<Engine::Transform>(
	Engine::State* pEngine, Entity entity
) {
	Archetype* pArchetype{ &pEngine->archetypes[entity.archetypeIndex] };

	ASSERT(
		entity.typeFlags & TransformComponent,
		"entity does not have correct component"
	);
	ASSERT(
		pArchetype->componentFlags & TransformComponent,
		"entity tied to wrong archetype"
	);

	size_t index{ pArchetype->uidToIndex[entity.uid] };
	return &pArchetype->transforms[index];
}

template<>
void Engine::setComponent(
	Engine::State* pEngine, Entity entity, Engine::Transform transform
) {
	Archetype* pArchetype{ &pEngine->archetypes[entity.archetypeIndex] };
	size_t index{ pArchetype->uidToIndex[entity.uid] };

	ASSERT(
		entity.typeFlags & TransformComponent,
		"entity does not have correct component"
	);
	ASSERT(
		pArchetype->componentFlags & TransformComponent,
		"entity tied to wrong archetype"
	);

	pArchetype->transforms[index] = transform;
}

template<>
void Engine::setComponent(
	Engine::State* pEngine, Entity entity, AssetManager::UID assetID
) {
	Archetype* pArchetype{ &pEngine->archetypes[entity.archetypeIndex] };
	size_t index{ pArchetype->uidToIndex[entity.uid] };

	ASSERT(
		entity.typeFlags & AssetComponent,
		"entity does not have correct component"
	);
	ASSERT(
		pArchetype->componentFlags & AssetComponent,
		"entity tied to wrong archetype"
	);

	pArchetype->assetIDs[index] = assetID;
}

Engine::Archetype Engine::createArchetype(
	pstd::Arena* pArena,
	Engine::ComponentTypeFlags componentFlags,
	size_t maxEntityCount
) {
	using namespace Engine;
	Archetype archetype{
		.componentFlags = componentFlags,
		.uidToIndex = pstd::createHashMap<UID, size_t>(pArena, maxEntityCount)
	};

	if (componentFlags & TransformComponent) {
		archetype.transforms =
			pstd::createArray<Transform>(pArena, maxEntityCount, 0);
	}
	if (componentFlags & AssetComponent) {
		archetype.assetIDs =
			pstd::createArray<AssetManager::UID>(pArena, maxEntityCount, 0);
	}

	return archetype;
}

pstd::String Engine::stringify(pstd::Arena* pArena, Entity entity) {
	return pstd::formatString(
		pArena,
		"Entity{ UID = %u, ComponentTypeFlags = %u, ArchetypeIndex = %u}\n",
		entity.uid,
		(uint32_t)entity.typeFlags,
		entity.archetypeIndex
	);
}

pstd::String Engine::stringify(pstd::Arena* pArena, Archetype archetype) {
	return pstd::formatString(
		pArena,
		"Archetype{ ComponentTypeFlags = %u, SparseMapCount = %u, "
		"SparseMapCapacity = %u, TransformsCount = %u, AssetsCount = %u}\n",
		archetype.componentFlags,
		archetype.uidToIndex.count,
		archetype.uidToIndex.capacity,
		archetype.transforms.count,
		archetype.assetIDs.count
	);
}
