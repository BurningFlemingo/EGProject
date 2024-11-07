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
		pstd::FixedArena gameArena;
	};
}  // namespace Game

GAME_API Game::State* Game::startup() {
	pstd::AllocationRegistry allocRegistry{ pstd::createAllocationRegistry() };
	pstd::FixedArena gameArena{
		pstd::allocateFixedArena(&allocRegistry, 1024)
	};

	pstd::CircularBuffer<int> cBuf{
		.allocation{ pstd::arenaAlloc<int>(&gameArena, 5) },
	};

	pstd::Vec2 myVec1{ .x = 1, .y = 2 };
	pstd::Vec2 myVec2{ .x = 2, .y = 3 };
	pstd::Vec2 vec{ myVec1 + myVec2 };
	pstd::Mat4 myMat{ pstd::getIdentityMatrix<4>() };

	pstd::Rot3 rotor{ pstd::calcRotor(pstd::UP, pstd::RIGHT, pstd::HALF_PI) };

	pstd::Vec3 myVec{ .y = 1.f, .z = 1.f };

	myVec = pstd::calcRotated(myVec, rotor);

	LOG_ERROR("my favorite number is %u cus its cool %i", 0, -23);

	pstd::pushBackOverwrite(&cBuf, 1);
	pstd::pushBackOverwrite(&cBuf, 2);
	pstd::pushBackOverwrite(&cBuf, 3);
	pstd::pushBackOverwrite(&cBuf, 4);
	pstd::pushBackOverwrite(&cBuf, 5);
	pstd::pushBackOverwrite(&cBuf, 6);

	float test1{ pstd::atanf(0) };
	test1 = pstd::atanf(1);
	test1 = pstd::atanf(-1);
	test1 = pstd::atanf(10000);
	test1 = pstd::atanf(-10000);
	test1 = pstd::atanf(0.00001);

	pstd::Allocation stateAllocation{ pstd::arenaAlloc<Game::State>(&gameArena
	) };
	Game::State* statePtr{ new (stateAllocation.block
	) Game::State{ .allocRegistry = allocRegistry, .gameArena = gameArena } };
	return statePtr;
}
GAME_API bool Game::update(State* state) {
	return true;
}
GAME_API void Game::shutdown(State* state) {}
