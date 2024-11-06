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

	extern "C" GAME_API State* startup();
	extern "C" GAME_API bool update(State* state);
	extern "C" GAME_API void shutdown(State* state);
}  // namespace Game
