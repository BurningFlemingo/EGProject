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

	using UID = size_t;
	struct ModelType;

	bool getPhysicalKeyDown(Engine::State* pAppState, InputCode keyCode);
	bool getVirtualKeyDown(Engine::State* pAppState, InputCode keyCode);

	UID createEntity(Engine::State* pApp);

	void addTransform(
		Engine::State* pApp, UID entityID, const Transform& transform
	);
	void addModel(Engine::State* pApp, UID entityID, pstd::String path);
	Transform getTransform(Engine::State* pApp, UID uid);
	void updateTransform(
		Engine::State* pApp, UID entityID, const Transform& transform
	);
}  // namespace Engine
