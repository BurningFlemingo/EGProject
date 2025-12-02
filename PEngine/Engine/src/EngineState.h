#pragma once

#include "Engine.h"
#include "STD/PMemory.h"
#include "STD/PString.h"
#include "STD/PArray.h"
#include "STD/PArena.h"
#include "Game.h"
#include "AssetLoader.h"
#include "Platforms/Event.h"
#include "STD/PHashMap.h"
#include "ECS.h"

struct GameDll {
	pstd::DllHandle handle;
	Game::API api;
	bool isValid;
	size_t lastWriteTime;
};

namespace Engine {

	struct KeyTransitionState {
		bool keyWasUp;
		bool keyWasDown;
	};

	static constexpr size_t maxEntityCount{ 1024 };

	struct State {
		pstd::Arena scratchArena;
		pstd::Arena subsystemArena;
		GameDll gameDll;
		Game::State* pGameState;
		pstd::String originalDllPath;
		const char* originalDllPathCString;
		bool isRunning;

		KeyTransitionState physicalKeyTransition[ncast<size_t>(KeyCode::COUNT)];
		KeyTransitionState virtualKeyTransition[ncast<size_t>(KeyCode::COUNT)];

		bool physicalKeyDown[ncast<size_t>(KeyCode::COUNT)];
		bool virtualKeyDown[ncast<size_t>(KeyCode::COUNT)];

		Cursor cursor;

		pstd::Array<Archetype> archetypes;

		pstd::HashMap<pstd::String, Entity> nameToEntity{};

		float lastFrameTime;
	};

}  // namespace Engine
