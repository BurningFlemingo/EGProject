#include "Core.h"
#include "Engine.h"
#include "Input.h"
#include "Logging.h"
#include "LoggingSetup.h"
#include "Game.h"
#include "AssetLoader.h"
#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "STD/PFileIO.h"
#include "STD/PString.h"
#include "STD/Memory.h"
#include "Platforms/Window.h"
#include "Renderer/Renderer.h"
#include "STD/PTime.h"

#include <new>

namespace {

	struct GameDll {
		pstd::DllHandle handle;
		Game::API api;
		bool isValid;
		size_t lastWriteTime;
	};

	GameDll loadGameDll(pstd::Arena scratchArena);
	void unloadGameDll(GameDll dll);
}  // namespace

namespace Application {

	struct State {
		pstd::AllocationRegistry allocationRegistry;
		pstd::Arena scratchArena;
		pstd::Arena subsystemArena;
		GameDll gameDll;
		Game::State* pGameState;
		pstd::String originalDllPath;
		const char* originalDllPathCString;
		bool isRunning;

		pstd::Array<bool, InputCode> virtualKeyState{};
		pstd::Array<bool, InputCode> physicalKeyState{};
	};

}  // namespace Application

Engine::Subsystems Engine::startup() {
	constexpr size_t scratchSize{ 1024 * 1024 };

	Console::startup();

	pstd::AllocationRegistry allocationRegistry{ pstd::createAllocationRegistry(
	) };

	pstd::Arena scratchArena{
		pstd::allocateArena(&allocationRegistry, scratchSize)
	};

	pstd::Arena subsystemArena{
		pstd::allocateArena(&allocationRegistry, scratchSize * 2)
	};

	GameDll gameDll{ loadGameDll(scratchArena) };

	Game::State* pGameState{ gameDll.api.startup() };

	pstd::String originalDllPath{ pstd::formatString(
		&scratchArena,
		"%mGame.%m",
		makeExeDirectoryPath(&scratchArena),
		pstd::getDllExtensionName()
	) };

	const char* originalDllPathCString{
		pstd::createCString(&scratchArena, originalDllPath)
	};

	Application::State* pApplicationState =
		new (pstd::alloc<Application::State>(&subsystemArena))
			Application::State{
				.allocationRegistry = allocationRegistry,
				.scratchArena = scratchArena,
				.subsystemArena = subsystemArena,
				.gameDll = gameDll,
				.pGameState = pGameState,
				.originalDllPath = originalDllPath,
				.originalDllPathCString = originalDllPathCString,
				.isRunning = true,
				.virtualKeyState = pstd::createArray<bool, InputCode>(
					&subsystemArena, ncast<size_t>(InputCode::COUNT)
				),
				.physicalKeyState = pstd::createArray<bool, InputCode>(
					&subsystemArena, ncast<size_t>(InputCode::COUNT)
				)
			};

	Platform::State* pPlatformState{
		Platform::startup(&subsystemArena, "window", 1920 / 2, 1080 / 2)
	};
	pstd::OBJ cubeOBJ{ pstd::loadOBJ(
		&pApplicationState->subsystemArena,
		pApplicationState->scratchArena,
		".\\assets\\models\\cube.obj"
	) };
	Renderer::State* pRendererState{ Renderer::startup(
		&subsystemArena, scratchArena, *pPlatformState, cubeOBJ
	) };

	int myThings[] = { 1, 2, 3, 4 };
	auto myArray{ pstd::createArray<int>(myThings) };

	return Engine::Subsystems{
		.pApplicationState = pApplicationState,
		.pRendererState = pRendererState,
		.pPlatformState = pPlatformState,
	};
}

void Engine::shutdown(const Subsystems& systems) {
	systems.pApplicationState->gameDll.api.shutdown(
		systems.pApplicationState->pGameState
	);

	Renderer::shutdown(systems.pRendererState);
	Platform::shutdown(systems.pPlatformState);
}

bool Engine::update(const Subsystems& systems) {
	Renderer::State* pRenderer{ systems.pRendererState };
	Platform::State* pPlatform{ systems.pPlatformState };
	Application::State* pApp{ systems.pApplicationState };

	if (pApp->isRunning && Platform::isRunning(pPlatform)) {
		Platform::update(pPlatform);

		Platform::Event event{};
		bool windowResized{};
		while (Platform::popEvent(pPlatform, &event)) {
			switch (event.type) {
				case Platform::EventType::key: {
					if (event.keyEvent.action == InputAction::PRESSED) {
						pApp->virtualKeyState[event.keyEvent.virtualCode] =
							true;
						pApp->physicalKeyState[event.keyEvent.physicalCode] =
							true;
					} else if (event.keyEvent.action == InputAction::RELEASED) {
						pApp->virtualKeyState[event.keyEvent.virtualCode] =
							false;
						pApp->physicalKeyState[event.keyEvent.physicalCode] =
							false;
					}
				} break;
				case Platform::EventType::window: {
					if (event.windowEvent.resized) {
						windowResized = true;
					}
				} break;
				default:
					break;
			}
		}
	}

	return pApp->isRunning;
}

void Engine::run(const Subsystems& systems) {
	Renderer::State* pRenderer{ systems.pRendererState };
	Platform::State* pPlatform{ systems.pPlatformState };
	Application::State* pApp{ systems.pApplicationState };

	float lastFrameTime{};
	while (pApp->isRunning) {
		float beginFrameTime{ ncast<float>(pstd::getTicks()) };
		float dT{ beginFrameTime - lastFrameTime };
		lastFrameTime = beginFrameTime;

		pstd::reset(&pApp->scratchArena);

		if (pstd::getLastFileWriteTime(pApp->originalDllPathCString) !=
			pApp->gameDll.lastWriteTime) {
			unloadGameDll(pApp->gameDll);
			pApp->gameDll = loadGameDll(pApp->scratchArena);
		}

		pApp->isRunning &= Engine::update(systems);
		pApp->isRunning &=
			pApp->gameDll.api.update(systems, pApp->pGameState, dT);

		Renderer::render(pRenderer, false);
	}
}

bool Engine::getPhysicalKeyDown(
	Application::State* pAppState, InputCode keyCode
) {
	bool isPressed{ pAppState->physicalKeyState[keyCode] };
	return isPressed;
}

bool Engine::getVirtualKeyDown(
	Application::State* pAppState, InputCode keyCode
) {
	bool isPressed{ pAppState->virtualKeyState[keyCode] };
	return isPressed;
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
}  // namespace
