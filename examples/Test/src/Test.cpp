#include "Engine.h"
#include "Game.h"
#include "Renderer.h"
#include "Logging.h"
#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "STD/PCircularBuffer.h"
#include "STD/PVector.h"
#include "STD/PMatrix.h"
#include "STD/PMath.h"
#include "STD/PTime.h"

#include <new>

namespace Game {
	struct State {
		pstd::Arena gameArena;
		Engine::UID cube1{};
		Engine::UID cube2{};

		pstd::Vec3 pos;
	};
}  // namespace Game

GAME_API Game::State* Game::startup(
	pstd::AllocationRegistry* pAllocRegistry, Engine::Subsystems subsystems
) {
	pstd::Arena gameArena{ pstd::allocateArena(pAllocRegistry, 1024) };

	Engine::UID cube1{ Engine::createEntity(subsystems.pEngine) };
	Engine::UID cube2{ Engine::createEntity(subsystems.pEngine) };
	Engine::Transform transform{ .pos = pstd::Vec3{ 0.f, 0.f, 3.f } };

	// heyyy
	Engine::addModel(subsystems.pEngine, cube1, ".\\assets\\models\\cube.obj");
	Engine::addTransform(subsystems.pEngine, cube1, transform);

	Engine::addModel(subsystems.pEngine, cube2, ".\\assets\\models\\cube.obj");
	Engine::addTransform(subsystems.pEngine, cube2, transform);

	Game::State* gameState{ pstd::alloc<Game::State>(&gameArena) };
	Game::State* statePtr{ new (gameState
	) Game::State{ .gameArena = gameArena, .cube1 = cube1, .cube2 = cube2 } };

	return statePtr;
}
GAME_API bool
	Game::update(Engine::Subsystems subsystems, State* state, float dTime) {
	Engine::State* pEngine{ subsystems.pEngine };

	float speed{ 0.03f * dTime };
	if (Engine::getPhysicalKeyDown(pEngine, InputCode::D)) {
		state->pos.x += speed;
	}
	if (Engine::getPhysicalKeyDown(pEngine, InputCode::A)) {
		state->pos.x -= speed;
	}
	if (Engine::getPhysicalKeyDown(pEngine, InputCode::W)) {
		state->pos.z += speed;
	}
	if (Engine::getPhysicalKeyDown(pEngine, InputCode::S)) {
		state->pos.z -= speed;
	}
	if (Engine::getVirtualKeyDown(pEngine, InputCode::SPACE)) {
		state->pos.y += speed * 5;
	}

	pstd::Rot3 rot{ pstd::calcRotor(
		{ 0, 1, 0 }, { 0, 0, 1 }, pstd::toRadians(0.0f) * pstd::getTicks()
	) };

	Engine::Transform transform1{ .pos = state->pos, .rot = rot };
	Engine::Transform transform2{ .pos =
									  state->pos + pstd::Vec3{ 5.f, 0.f, 0.f },
								  .rot = rot };

	Engine::updateTransform(pEngine, state->cube1, transform1);
	Engine::updateTransform(pEngine, state->cube2, transform2);

	return !Engine::getVirtualKeyDown(pEngine, InputCode::TAB);
}
GAME_API void Game::shutdown(State* state) {}
