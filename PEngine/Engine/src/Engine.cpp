#include "Core.h"
#include "Engine.h"
#include "Input.h"
#include "Cursor.h"
#include "Logging.h"
#include "LoggingSetup.h"
#include "Game.h"
#include "GameObject.h"
#include "Platforms/Event.h"
#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "STD/PFileIO.h"
#include "STD/PString.h"
#include "STD/Memory.h"
#include "Platforms/Window.h"
#include "Renderer/Renderer.h"
#include "STD/PTime.h"
#include "AssetLoader.h"
#include "EngineState.h"
#include "STD/PMath.h"

#include <new>

namespace {
	GameDll loadGameDll(pstd::Arena scratchArena);
	void unloadGameDll(GameDll dll);
	void resetKeyState(Engine::State* pEngine);
}  // namespace

Engine::Subsystems Engine::startup(pstd::AllocationRegistry* pAllocRegistry) {
	constexpr size_t scratchSize{ 1024 * 1024 * 8 };

	Console::startup();

	pstd::Arena scratchArena{
		pstd::allocateArena(pAllocRegistry, scratchSize)
	};

	pstd::Arena subsystemArena{
		pstd::allocateArena(pAllocRegistry, scratchSize * 2)
	};

	GameDll gameDll{ loadGameDll(scratchArena) };

	pstd::String originalDllPath{ pstd::formatString(
		&scratchArena,
		"%mGame.%m",
		makeExeDirectoryPath(&scratchArena),
		pstd::getDllExtensionName()
	) };

	const char* originalDllPathCString{
		pstd::createCString(&scratchArena, originalDllPath)
	};

	auto models{ pstd::createArray<Engine::MeshData>(&subsystemArena, 10, 0) };
	auto transforms{
		pstd::createArray<Engine::Transform>(&subsystemArena, 10, 0)
	};
	auto entityUIDs{ pstd::createArray<Engine::UID>(&subsystemArena, 10, 0) };

	Platform::State* pPlatform{
		Platform::startup(&subsystemArena, "window", 1920 / 2, 1080 / 2)
	};

	Renderer::State* pRenderer{ Renderer::startup(
		pAllocRegistry, &subsystemArena, scratchArena, *pPlatform
	) };

	Engine::State* pEngine =
		new (pstd::alloc<Engine::State>(&subsystemArena)) Engine::State{
			.scratchArena = scratchArena,
			.subsystemArena = subsystemArena,
			.gameDll = gameDll,
			.originalDllPath = originalDllPath,
			.originalDllPathCString = originalDllPathCString,
			.isRunning = true,
			.models = models,
			.transforms = transforms,
			.entityUIDs = entityUIDs,
		};

	Engine::Subsystems subsystems{
		.pEngine = pEngine,
		.pRenderer = pRenderer,
		.pPlatform = pPlatform,
	};

	Game::State* pGameState{
		pEngine->gameDll.api.startup(pAllocRegistry, subsystems)
	};
	pEngine->pGameState = pGameState;

	Renderer::setModels(pRenderer, pEngine->scratchArena, pEngine->models);

	return subsystems;
}

void Engine::shutdown(const Subsystems& systems) {
	systems.pEngine->gameDll.api.shutdown(systems.pEngine->pGameState);
	Renderer::shutdown(systems.pRenderer);
	Platform::shutdown(systems.pPlatform);
}

bool Engine::update(
	pstd::AllocationRegistry* pAllocRegistry, const Subsystems& systems
) {
	Renderer::State* pRenderer{ systems.pRenderer };
	Platform::State* pPlatform{ systems.pPlatform };
	Engine::State* pEngine{ systems.pEngine };

	pEngine->cursor.dx = 0;
	pEngine->cursor.dy = 0;

	if (pEngine->isRunning && Platform::isRunning(pPlatform)) {
		Platform::update(pPlatform);

		memset(
			pEngine->virtualKeyTransition,
			0,
			sizeof(pEngine->virtualKeyTransition)
		);
		memset(
			pEngine->physicalKeyTransition,
			0,
			sizeof(pEngine->physicalKeyTransition)
		);

		Platform::Event event{};
		bool windowResized{};
		while (Platform::popEvent(pPlatform, &event)) {
			switch (event.type) {
			case Platform::EventType::key: {
				Platform::CompressedKeyState keyState{ event.keyEvent.state };

				size_t virtualCode{ ncast<size_t>(event.keyEvent.virtualCode) };
				size_t physicalCode{ ncast<size_t>(event.keyEvent.physicalCode
				) };

				pEngine->virtualKeyTransition[virtualCode].keyWasUp |=
					keyState.wasUp;
				pEngine->virtualKeyTransition[virtualCode].keyWasDown |=
					!keyState.wasUp;

				pEngine->physicalKeyTransition[physicalCode].keyWasUp |=
					keyState.wasUp;
				pEngine->physicalKeyTransition[physicalCode].keyWasDown |=
					!keyState.wasUp;

				pEngine->virtualKeyDown[virtualCode] = keyState.isDown;
				pEngine->physicalKeyDown[physicalCode] = keyState.isDown;
			} break;
			case Platform::EventType::cursor: {
				pEngine->cursor.dx += event.cursorEvent.dx;
				pEngine->cursor.dy += event.cursorEvent.dy;
			} break;
			case Platform::EventType::window: {
				if (event.windowEvent.resized) {
					windowResized = true;
				}
				if (event.windowEvent.lostFocus) {
					resetKeyState(pEngine);
				}
			} break;
			default:
				break;
			}
		}
	}

	return pEngine->isRunning;
}

Engine::UID Engine::createEntity(Engine::State* pEngine) {
	size_t uid{ pEngine->entityUIDs.count };
	pstd::pushBack(&pEngine->entityUIDs, uid);
	return uid;
}

void Engine::addTransform(
	Engine::State* pEngine, const UID entityID, const Transform& transform
) {
	pEngine->transforms.count =
		max(pEngine->transforms.count, pEngine->entityUIDs.count);
	pEngine->transforms[entityID] = transform;
}

void Engine::addModel(
	Engine::State* pEngine, const UID entityID, pstd::String path
) {
	Engine::MeshData model{
		Engine::loadMesh(&pEngine->subsystemArena, pEngine->scratchArena, path)
	};

	pEngine->models.count =
		max(pEngine->models.count, pEngine->entityUIDs.count);

	pEngine->models[entityID] = model;
}

Engine::Transform Engine::getTransform(Engine::State* pEngine, UID uid) {
	return pEngine->transforms[uid];
}

Engine::Cursor Engine::getCursor(Engine::State* pEngine) {
	return pEngine->cursor;
}

void Engine::updateTransform(
	Engine::State* pEngine, UID uid, const Transform& transform
) {
	pEngine->transforms.count =
		max(pEngine->transforms.count, pEngine->entityUIDs.count);
	pEngine->transforms[uid] = transform;
}

bool Engine::tick(
	pstd::AllocationRegistry* pAllocRegistry, const Subsystems& subsystems
) {
	Renderer::State* pRenderer{ subsystems.pRenderer };
	Platform::State* pPlatform{ subsystems.pPlatform };
	Engine::State* pEngine{ subsystems.pEngine };

	if (pEngine->isRunning) {
		float beginFrameTime{ ncast<float>(pstd::getTicks()) };
		float dT{ beginFrameTime - pEngine->lastFrameTime };
		pEngine->lastFrameTime = beginFrameTime;

		// LOG_INFO("fps: %f\n", 1.f / (dT / 1000.f));

		pstd::reset(&pEngine->scratchArena);

		if (pstd::getLastFileWriteTime(pEngine->originalDllPathCString) !=
			pEngine->gameDll.lastWriteTime) {
			unloadGameDll(pEngine->gameDll);
			pEngine->gameDll = loadGameDll(pEngine->scratchArena);
		}

		pEngine->isRunning &= Engine::update(pAllocRegistry, subsystems);

		pEngine->isRunning &=
			pEngine->gameDll.api.update(subsystems, pEngine->pGameState, dT);

		Renderer::setTransforms(pRenderer, pEngine->transforms);

		Renderer::render(pRenderer, false);
	}

	return pEngine->isRunning;
}

Engine::KeyState Engine::getPKeyState(Engine::State* pEngine, KeyCode keyCode) {
	KeyTransitionState transitionState{
		pEngine->physicalKeyTransition[(size_t)keyCode]
	};
	bool isDown{ pEngine->physicalKeyDown[(size_t)keyCode] };

	return Engine::KeyState{
		.wasPressed = transitionState.keyWasUp,
		.wasReleased = transitionState.keyWasDown,
		.isDown = isDown,
		.isUp = !isDown,
	};
}

Engine::KeyState Engine::getVKeyState(Engine::State* pEngine, KeyCode keyCode) {
	KeyTransitionState transitionState{
		pEngine->virtualKeyTransition[(size_t)keyCode]
	};
	bool isDown{ pEngine->virtualKeyDown[(size_t)keyCode] };

	return Engine::KeyState{
		.wasPressed = transitionState.keyWasUp,
		.wasReleased = transitionState.keyWasDown,
		.isDown = isDown,
		.isUp = !isDown,
	};
}
namespace {
	GameDll loadGameDll(pstd::Arena scratchArena) {
		static uint32_t loadedDllSlot{};

		const pstd::String originalDllName{ pstd::createString("Game") };

		pstd::String loadedDllPath{ pstd::formatString(
			&scratchArena,
			"%mGame_Loaded_%u.%m",
			makeExeDirectoryPath(&scratchArena),
			loadedDllSlot,
			pstd::getDllExtensionName()
		) };

		uint32_t unloadedDllSlot{ (loadedDllSlot + 1) % 2 };

		pstd::String toLoadDllPath{ pstd::formatString(
			&scratchArena,
			"%mGame_Loaded_%u.%m",
			makeExeDirectoryPath(&scratchArena),
			unloadedDllSlot,
			pstd::getDllExtensionName()
		) };

		pstd::String originalDllPath{ pstd::formatString(
			&scratchArena,
			"%mGame.%m",
			makeExeDirectoryPath(&scratchArena),
			pstd::getDllExtensionName()
		) };

		pstd::copyFile(
			pstd::createCString(&scratchArena, toLoadDllPath),
			pstd::createCString(&scratchArena, originalDllPath),
			true
		);

		pstd::DllHandle gameHandle{
			pstd::loadDll(pstd::createCString(&scratchArena, toLoadDllPath))
		};
		loadedDllSlot = unloadedDllSlot;

		Game::API gameAPI{
			.startup = (Game::API::Startup
			)pstd::findDllFunction(gameHandle, "startup"),
			.update =
				(Game::API::Update)pstd::findDllFunction(gameHandle, "update"),
			.shutdown = (Game::API::Shutdown
			)pstd::findDllFunction(gameHandle, "shutdown"),
		};

		bool isValid{ gameAPI.shutdown && gameAPI.update && gameAPI.shutdown };

		GameDll res{ .handle = gameHandle,
					 .api = gameAPI,
					 .isValid = isValid,
					 .lastWriteTime = pstd::getLastFileWriteTime(
						 pstd::createCString(&scratchArena, originalDllPath)
					 ) };
		return res;
	}
	void unloadGameDll(GameDll dll) {
		if (dll.handle) {
			pstd::unloadDll(dll.handle);
		}
	}

	void resetKeyState(Engine::State* pEngine) {
		memset(pEngine->physicalKeyDown, 0, sizeof(pEngine->physicalKeyDown));
		memset(pEngine->virtualKeyDown, 0, sizeof(pEngine->virtualKeyDown));
	}
}  // namespace
