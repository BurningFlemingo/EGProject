#include "Engine.h"
#include "Logging.h"
#include "LoggingSetup.h"
#include "Game.h"
#include "Core/PArena.h"
#include "Core/PMemory.h"
#include "Core/PFileIO.h"
#include "Core/PString.h"
#include "Core/Memory.h"
#include <Windows.h>

namespace {
	struct GameDll {
		pstd::DllHandle handle;
		Game::API api;
		bool isValid;
		size_t lastWriteTime;
	};

	GameDll loadGameDll(pstd::Arena scratchArena);
	void unloadGameDll(GameDll dll);

	PE::State* engineState;
}  // namespace

int main() {
	Console::startup();

	pstd::AllocationRegistry allocationRegistry{ pstd::createAllocationRegistry(
	) };
	constexpr size_t scratchSize{ 1024 * 1024 };

	pstd::Arena scratchArena{
		pstd::allocateArena(&allocationRegistry, scratchSize)
	};

	pstd::Arena engineArena{ pstd::allocateArena(
		&allocationRegistry, PE::getSizeofState() + scratchSize
	) };

	engineState = PE::startup(&engineArena, scratchArena);

	GameDll gameDll{ loadGameDll(scratchArena) };
	Game::State* gameState{ gameDll.api.startup() };

	pstd::String originalDllPath{ pstd::formatString(
		&scratchArena,
		"%mGame.%m",
		makeExeDirectoryPath(&scratchArena),
		pstd::getDllExtensionName()
	) };

	bool isRunning{ true };
	const char* originalDllPathCString{
		pstd::createCString(&scratchArena, originalDllPath)
	};
	while (isRunning) {
		if (pstd::getLastFileWriteTime(originalDllPathCString) !=
			gameDll.lastWriteTime) {
			unloadGameDll(gameDll);
			gameDll = loadGameDll(scratchArena);
		}

		isRunning &= PE::update(engineState);
		isRunning &= gameDll.api.update(gameState);
	}

	gameDll.api.shutdown(gameState);
	PE::shutdown(engineState);
}

namespace {

	GameDll loadGameDll(pstd::Arena scratchArena) {
		static uint32_t loadedDllSlot{};

		constexpr pstd::String originalDllName{ pstd::createString("Game") };

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
					 .lastWriteTime =
						 pstd::getLastFileWriteTime(originalDllPath.buffer) };
		return res;
	}
	void unloadGameDll(GameDll dll) {
		if (dll.handle) {
			pstd::unloadDll(dll.handle);
		}
	}
}  // namespace
