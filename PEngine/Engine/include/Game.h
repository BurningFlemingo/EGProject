#pragma once
#include "GameAPI.h"
#include "Engine.h"

namespace Game {
	struct State;

	struct API {
		using Startup = State* (*)();
		using Update = bool (*)(Engine::Subsystems subsystems, State* state);
		using Shutdown = void (*)(State* state);

		Startup startup;
		Update update;
		Shutdown shutdown;
	};

	GAME_API State* startup();
	GAME_API bool update(Engine::Subsystems subsystems, State* state);
	GAME_API void shutdown(State* state);
}  // namespace Game
