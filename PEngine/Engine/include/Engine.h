#pragma once
#include "STD/PMatrix.h"
#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "STD/PFileIO.h"
#include "GameObject.h"

#include "Input.h"

#include "Renderer.h"

namespace Platform {
	struct State;
}

namespace Engine {
	struct State;

	struct Subsystems {
		Engine::State* pEngine;
		Renderer::State* pRenderer;
		Platform::State* pPlatform;
	};

	struct Cursor {
		float dx;
		float dy;
	};

	struct Camera {
		Transform transform;
	};

	using UID = size_t;
	struct ModelType;

	// virtual key state
	KeyState getVKeyState(Engine::State* pEngineState, KeyCode keyCode);
	// physical key state
	KeyState getPKeyState(Engine::State* pEngineState, KeyCode keyCode);

	UID createEntity(Engine::State* pEngine);

	void addTransform(
		Engine::State* pEngine, UID entityID, const Transform& transform
	);
	void addModel(Engine::State* pEngine, UID entityID, pstd::String path);

	Engine::Cursor getCursor(Engine::State* PEngine);

	Transform getTransform(Engine::State* pEngine, UID uid);
	void updateTransform(
		Engine::State* pEngine, UID entityID, const Transform& transform
	);
}  // namespace Engine
