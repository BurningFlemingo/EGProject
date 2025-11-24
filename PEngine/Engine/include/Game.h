#pragma once
#include "GameAPI.h"
#include "Engine.h"
#include "STD/PMemory.h"

namespace Game {
	struct State;

	struct API {
		using Startup = State* (*)(pstd::AllocationRegistry* pAllocRegistry,
								   Engine::Subsystems subsystems);
		using Update =
			bool (*)(Engine::Subsystems subsystems, State* state, float dTime);
		using Shutdown = void (*)(State* state);

		Startup startup;
		Update update;
		Shutdown shutdown;
	};

	GAME_API State* startup(
		pstd::AllocationRegistry* pAllocRegistry, Engine::Subsystems subsystems
	);
	GAME_API bool
		update(Engine::Subsystems subsystems, State* state, float dTime);
	GAME_API void shutdown(State* state);
}  // namespace Game
