#pragma once
#include "GameAPI.h"

namespace Game {
	struct State;

	struct API {
		using Startup = State* (*)();
		using Update = bool (*)(State* state);
		using Shutdown = void (*)(State* state);

		Startup startup;
		Update update;
		Shutdown shutdown;
	};

	GAME_API State* startup();
	GAME_API bool update(State* state);
	GAME_API void shutdown(State* state);
}  // namespace Game
