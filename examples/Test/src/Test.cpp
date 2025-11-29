#include "Camera.h"
#include "Cursor.h"
#include "Game.h"
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
		Renderer::Camera camera;

		pstd::Vec3 pos;

		float pitch;
		float yaw;

		bool inMenu;
	};
}  // namespace Game

using namespace Engine;

GAME_API Game::State* Game::startup(
	pstd::AllocationRegistry* pAllocRegistry, Subsystems subsystems
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

	Renderer::Camera camera{
		.transform = Transform{ .pos{ 0, 0, 5 }, .rot = { 0, 0, 0, 1 } },
		.fovRadians = pstd::toRadians(90),
		.nearPlane = 0.1,
		.farPlane = 100.0,
	};
	Renderer::setCamera(subsystems.pRenderer, camera);

	Game::State* gameState{ pstd::alloc<Game::State>(&gameArena) };
	Game::State* statePtr{ new (gameState) Game::State{ .gameArena = gameArena,
														.cube1 = cube1,
														.cube2 = cube2,
														.camera = camera } };

	return statePtr;
}
GAME_API bool Game::update(Subsystems subsystems, State* state, float dTime) {
	Engine::State* pEngine{ subsystems.pEngine };
	Platform::State* pPlatform{ subsystems.pPlatform };

	float speed{ 0.03f * dTime };
	float sensitivity{ 0.003f };

	pstd::Vec3 movement{};
	if (getPKeyState(pEngine, KeyCode::D).isDown) {
		movement.x += speed;
	}
	if (getPKeyState(pEngine, KeyCode::A).isDown) {
		movement.x -= speed;
	}
	if (getPKeyState(pEngine, KeyCode::W).isDown) {
		movement.z += speed;
	}
	if (getPKeyState(pEngine, KeyCode::S).isDown) {
		movement.z -= speed;
	}
	if (getPKeyState(pEngine, KeyCode::SPACE).isDown) {
		movement.y += speed * 5;
	}
	if (getVKeyState(pEngine, KeyCode::ESC).wasPressed) {
		if (!state->inMenu) {
			Platform::showCursor(pPlatform);
			Platform::releaseCursor(pPlatform);
		} else {
			Platform::hideCursor(pPlatform);
			Platform::captureCursor(pPlatform);
		}
		state->inMenu = !state->inMenu;
	}

	if (state->inMenu) {
		return !Engine::getVKeyState(pEngine, KeyCode::TAB).isDown;
	}

	Cursor cursor{ getCursor(pEngine) };
	state->pitch += cursor.dy * sensitivity;
	state->yaw += cursor.dx * sensitivity;

	pstd::Rot3 pitchRot{
		pstd::calcRotor({ 0, 0, 1 }, { 0, 1, 0 }, state->pitch)
	};
	pstd::Rot3 yawRot{ pstd::calcRotor({ 0, 0, 1 }, { 1, 0, 0 }, state->yaw) };

	pstd::Rot3 rot{ pstd::composeRotor(yawRot, pitchRot) };

	state->pos -= pstd::calcRotated(movement, rot);

	state->camera.transform = Transform{ .pos = state->pos, .rot = rot };

	Transform transform1{ .pos = { 0, 0, 1 } };
	Transform transform2{ .pos = { 2, 0, 1 } };

	updateTransform(pEngine, state->cube1, transform1);
	updateTransform(pEngine, state->cube2, transform2);

	return !Engine::getVKeyState(pEngine, KeyCode::TAB).isDown;
}
GAME_API void Game::shutdown(State* state) {}
