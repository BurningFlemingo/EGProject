#include "Engine/Game.h"
#include "Engine/Logging.h"
#include "Core/PArena.h"
#include "Core/PMemory.h"
#include "Core/PCircularBuffer.h"
#include "Core/PVector.h"
#include "Core/PMatrix.h"
#include "Core/PMath.h"

#include <new>

namespace Game {
	struct State {
		pstd::AllocationRegistry allocRegistry;
		pstd::Arena gameArena;
	};
}  // namespace Game

GAME_API Game::State* Game::startup() {
	pstd::AllocationRegistry allocRegistry{ pstd::createAllocationRegistry() };
	pstd::Arena gameArena{ pstd::allocateArena(&allocRegistry, 1024) };

	Game::State* gameState{ pstd::alloc<Game::State>(&gameArena) };
	Game::State* statePtr{ new (gameState
	) Game::State{ .allocRegistry = allocRegistry, .gameArena = gameArena } };
	return statePtr;
}
GAME_API bool Game::update(State* state) {
	return true;
}
GAME_API void Game::shutdown(State* state) {}
