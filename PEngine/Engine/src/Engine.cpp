#include "AssetManager.h"
#include "Assets.h"
#include "Core.h"
#include "Engine.h"
#include "ECS.h"
#include "Input.h"
#include "Cursor.h"
#include "Logging.h"
#include "LoggingSetup.h"
#include "Game.h"
#include "GameObject.h"
#include "Platforms/Event.h"
#include "STD/PArena.h"
#include "STD/PArray.h"
#include "STD/PHashMap.h"
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

	Platform::State* pPlatform{
		Platform::startup(&subsystemArena, "window", 1920 / 2, 1080 / 2)
	};

	Renderer::State* pRenderer{ Renderer::startup(
		pAllocRegistry, &subsystemArena, scratchArena, *pPlatform
	) };

	AssetManager::State* pAssetManager{
		AssetManager::startup(&subsystemArena, 1, 1024 * 1024 * 4)
	};

	Archetype renderableArchetype{ createArchetype(
		&subsystemArena,
		TransformComponent | AssetComponent,
		Engine::maxEntityCount
	) };

	pstd::Array<Archetype> archetypes{ pstd::createArray<Archetype>(
		&subsystemArena, Engine::maxArchetypeCount, 0
	) };

	pstd::pushBack(&archetypes, renderableArchetype);

	auto nameToEntity{ pstd::createHashMap<pstd::String, Entity>(
		&subsystemArena, Engine::maxEntityCount
	) };

	Engine::State* pEngine =
		new (pstd::alloc<Engine::State>(&subsystemArena)) Engine::State{
			.scratchArena = scratchArena,
			.subsystemArena = subsystemArena,
			.gameDll = gameDll,
			.originalDllPath = originalDllPath,
			.isRunning = true,
			.archetypes = archetypes,
			.nameToEntity = nameToEntity,
		};

	Engine::Subsystems subsystems{
		.pEngine = pEngine,
		.pAssetManager = pAssetManager,
		.pRenderer = pRenderer,
		.pPlatform = pPlatform,
	};

	Game::State* pGameState{
		pEngine->gameDll.api.startup(pAllocRegistry, subsystems)
	};

	pEngine->pGameState = pGameState;

	auto assetIDs{ pstd::createArray<AssetManager::UID>(
		&pEngine->scratchArena, Engine::maxEntityCount, 0
	) };

	for (size_t i{}; i < pEngine->archetypes.count; i++) {
		Archetype archetype{ pEngine->archetypes[i] };
		uint32_t renderableFlags{ TransformComponent | AssetComponent };
		if ((archetype.componentFlags & renderableFlags) == renderableFlags) {
			for (int j{}; j < archetype.assetIDs.count; j++) {
				UID uid{ archetype.assetIDs[j] };
				pstd::pushBack(&assetIDs, uid);
			}
		}
	}
	Renderer::setModels(
		pRenderer, pAssetManager, pEngine->scratchArena, assetIDs
	);

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

Engine::Cursor Engine::getCursor(Engine::State* pEngine) {
	return pEngine->cursor;
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

		LOG_INFO("fps: %f\n", 1.f / (dT / 1000.f));

		pstd::reset(&pEngine->scratchArena);

		// if (pstd::getLastFileWriteTime(pEngine->originalDllPathCString) !=
		// 	pEngine->gameDll.lastWriteTime) {
		// 	LOG_INFO(
		// 		"last write time: %u, current write time %u\n",
		// 		pstd::getLastFileWriteTime(pEngine->originalDllPathCString),
		// 		pEngine->gameDll.lastWriteTime
		// 	);
		// 	unloadGameDll(pEngine->gameDll);
		// 	pEngine->gameDll = loadGameDll(pEngine->scratchArena);
		// }
		pEngine->isRunning &= Engine::update(pAllocRegistry, subsystems);

		pEngine->isRunning &=
			pEngine->gameDll.api.update(subsystems, pEngine->pGameState, dT);

		auto transforms{ pstd::createArray<Transform>(
			&pEngine->scratchArena, Engine::maxEntityCount, 0
		) };
		for (size_t i{}; i < pEngine->archetypes.count; i++) {
			Archetype archetype{ pEngine->archetypes[i] };
			uint32_t renderableFlags{ TransformComponent | AssetComponent };
			if ((archetype.componentFlags & renderableFlags) ==
				renderableFlags) {
				for (int j{}; j < archetype.transforms.count; j++) {
					pstd::pushBack(&transforms, archetype.transforms[j]);
				}
			}
		}

		Renderer::setTransforms(pRenderer, transforms);

		Renderer::render(pRenderer, *pPlatform, false);
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
