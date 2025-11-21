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
		pstd::AllocationRegistry allocRegistry;
		pstd::Arena gameArena;

		pstd::Vec3 pos;
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
GAME_API bool Game::update(Engine::Subsystems subsystems, State* state) {
	float speed{ 0.3 };
	if (Engine::getKeyDown(subsystems.pApplicationState, (InputCode)'S')) {
		state->pos.x += speed;
	}
	if (Engine::getKeyDown(subsystems.pApplicationState, (InputCode)'A')) {
		state->pos.x -= speed;
	}
	if (Engine::getKeyDown(subsystems.pApplicationState, (InputCode)'W')) {
		state->pos.z += speed;
	}
	if (Engine::getKeyDown(subsystems.pApplicationState, (InputCode)'R')) {
		state->pos.z -= speed;
	}
	float ar{ 1920.0 / 1080.0 };
	pstd::Mat4 perspProjMatrix{
		pstd::calcPerspectiveMatrix(pstd::toRadians(90), ar, 0.001, 25)
	};

	pstd::Mat4 viewMatrix{
		pstd::calcLookAtMatrix({ 0.f, 0.f, 0.f }, { 0.f, 0.f, 1.f }, pstd::UP)
	};

	pstd::Rot3 rot{ pstd::calcRotor(
		{ 0, 1, 0 }, { 0, 0, 1 }, pstd::toRadians(0.1f) * pstd::getTicks()
	) };

	pstd::Mat4 rotMat{ pstd::calcRotationMatrix<4>(rot) };

	pstd::Mat4 modelMat{
		pstd::calcTranlsated(pstd::getIdentityMatrix<4>(), state->pos)
	};

	perspProjMatrix = perspProjMatrix * viewMatrix * modelMat * rotMat;
	Renderer::setMVPMatrix(subsystems.pRendererState, perspProjMatrix);

	return true;
}
GAME_API void Game::shutdown(State* state) {}
