#include "ECS.h"
#include "EngineState.h"
#include "Engine.h"

Engine::Entity Engine::getEntity(Engine::State* pEngine, pstd::String name) {
	return pEngine->nameToEntity[name];
}

Engine::Entity Engine::createEntity(
	Engine::State* pEngine, pstd::String name, ComponentTypeFlags componentFlags
) {
	Entity entity{ .uid = pstd::hash(name), .typeFlags = componentFlags };
	pEngine->nameToEntity[name] = entity;

	for (size_t i{}; i < pEngine->archetypes.count; i++) {
		Archetype* pArchetype{ &pEngine->archetypes[i] };
		if (pArchetype->componentFlags == componentFlags) {
			size_t index{ pArchetype->uidToIndex.count };
			pArchetype->uidToIndex[entity.uid] = index;

			if (componentFlags & TransformComponent) {
				pstd::pushBack(&pArchetype->transforms, {});
			}
			if (componentFlags & ModelComponent) {
				pstd::pushBack(&pArchetype->models, {});
			}

			return entity;
		}
	}

	Archetype archetype{ createArchetype(
		&pEngine->subsystemArena, componentFlags, Engine::maxEntityCount
	) };
	pstd::pushBack(&pEngine->archetypes, archetype);

	return createEntity(pEngine, name, componentFlags);
}

template<>
Engine::Transform Engine::getComponent<Engine::Transform>(
	Engine::State* pEngine, Entity entity
) {
	for (size_t i{}; i < pEngine->archetypes.count; i++) {
		Archetype* pArchetype{ &pEngine->archetypes[i] };
		if (pArchetype->componentFlags == entity.typeFlags) {
			size_t index{ pArchetype->uidToIndex[entity.uid] };
			return pArchetype->transforms[index];
		}
	}
	ASSERT(false, "entity archetype not initialized");
	return {};
}

template<>
void Engine::setComponent(
	Engine::State* pEngine, Entity entity, Engine::Transform transform
) {
	for (size_t i{}; i < pEngine->archetypes.count; i++) {
		Archetype* pArchetype{ &pEngine->archetypes[i] };
		if (pArchetype->componentFlags == entity.typeFlags) {
			size_t index{ pArchetype->uidToIndex[entity.uid] };
			pArchetype->transforms[index] = transform;
			return;
		}
	}

	ASSERT(false, "entity archetype not initialized");
}

template<>
void Engine::setComponent(
	Engine::State* pEngine, Entity entity, Engine::Model model
) {
	Engine::MeshData meshData{
		Engine::loadMesh(&pEngine->subsystemArena, pEngine->scratchArena, model)
	};
	for (size_t i{}; i < pEngine->archetypes.count; i++) {
		Archetype* pArchetype{ &pEngine->archetypes[i] };
		if (pArchetype->componentFlags == entity.typeFlags) {
			size_t index{ pArchetype->uidToIndex[entity.uid] };
			pArchetype->models[index] = meshData;
			return;
		}
	}

	ASSERT(false, "entity archetype not initialized");
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
	if (componentFlags & ModelComponent) {
		archetype.models =
			pstd::createArray<MeshData>(pArena, maxEntityCount, 0);
	}

	return archetype;
}

pstd::Array<Engine::Archetype*> Engine::getMatchingArchetypes(
	pstd::Arena* pArena,
	pstd::Array<Archetype>* pArchetypes,
	uint32_t componentFlags
) {
	auto matchedArchetypes{
		pstd::createArray<Archetype*>(pArena, Engine::maxArchetypeCount, 0)
	};

	for (size_t i{}; i < pArchetypes->count; i++) {
		Archetype* pArchetype{ &(*pArchetypes)[i] };
		if ((pArchetype->componentFlags & componentFlags) == componentFlags) {
			for (int j{}; j < pArchetype->uidToIndex.count; j++) {
				pstd::pushBack(&matchedArchetypes, pArchetype);
			}
		}
	}

	return matchedArchetypes;
}
