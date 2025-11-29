#pragma once

#include "STD/PMemory.h"
#include "STD/PString.h"
#include "STD/PArray.h"
#include "STD/PArena.h"
#include "Game.h"
#include "AssetLoader.h"
#include "Platforms/Event.h"

struct GameDll {
	pstd::DllHandle handle;
	Game::API api;
	bool isValid;
	size_t lastWriteTime;
};

namespace Engine {

	struct State {
		pstd::AllocationRegistry allocationRegistry;
		pstd::Arena scratchArena;
		pstd::Arena subsystemArena;
		GameDll gameDll;
		pstd::String originalDllPath;
		const char* originalDllPathCString;
		bool isRunning;

		Platform::CompressedKeyState
			virtualKeyState[ncast<size_t>(KeyCode::COUNT)]{};
		Platform::CompressedKeyState
			physicalKeyState[ncast<size_t>(KeyCode::COUNT)]{};

		Cursor cursor;

		pstd::Array<Engine::MeshData> models;
		pstd::Array<Engine::Transform> transforms;
		pstd::Array<Engine::UID> entityUIDs;
	};

}  // namespace Engine
