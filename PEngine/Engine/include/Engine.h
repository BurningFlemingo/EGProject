#pragma once
#include "STD/PMatrix.h"
#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "STD/PFileIO.h"
#include "GameObject.h"

#include "Input.h"

namespace Platform {
	struct State;
}

namespace Renderer {
	struct State;
}

namespace AssetManager {
	struct State;
}

namespace Engine {
	struct State;

	struct Subsystems {
		Engine::State* pEngine;
		AssetManager::State* pAssetManager;
		Renderer::State* pRenderer;
		Platform::State* pPlatform;
	};

	using UID = size_t;

	struct Cursor {
		float dx;
		float dy;
	};

	using ComponentTypeFlags = uint32_t;
	enum ComponentType : uint32_t {
		TransformComponent = 0b1,
		AssetComponent = 0b10,
		RigidbodyComponent = 0b100
	};

	struct Entity {
		size_t uid;
		ComponentTypeFlags typeFlags;
		size_t archetypeIndex;

		bool operator==(Entity other) { return uid == other.uid; }
	};

	// virtual key state
	KeyState getVKeyState(Engine::State* pEngineState, KeyCode keyCode);
	// physical key state
	KeyState getPKeyState(Engine::State* pEngineState, KeyCode keyCode);

	Engine::Cursor getCursor(Engine::State* pEngine);

	Entity createEntity(
		Engine::State* pEngine,
		pstd::String name,
		ComponentTypeFlags componentTypes
	);
	Entity
		createEntity(Engine::State* pEngine, ComponentTypeFlags componentTypes);

	Entity getEntity(Engine::State* pEngine, pstd::String name);

	// storing this pointer is dangerous, dont do it, you've been warned
	template<typename T>
	T* getComponent(Engine::State* pEngine, Entity entity);

	template<typename T>
	void setComponent(Engine::State* pEngine, Entity entity, T component);
}  // namespace Engine
