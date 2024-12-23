#include "Engine/include/Game.h"
#include "Engine/include/Logging.h"
#include "STD/include/PArena.h"
#include "STD/include/PMemory.h"
#include "STD/include/PCircularBuffer.h"
#include "STD/include/PVector.h"
#include "STD/include/PMatrix.h"
#include "STD/include/PMath.h"

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
	pstd::ArenaFrame frame{ .pArena = &gameArena };

	pstd::Allocation stateAllocation{ pstd::alloc<Game::State>(&frame) };
	Game::State* statePtr{ new (stateAllocation.block
	) Game::State{ .allocRegistry = allocRegistry, .gameArena = gameArena } };
	return statePtr;
}
GAME_API bool Game::update(State* state) {
	return true;
}
GAME_API void Game::shutdown(State* state) {}
