#include "Engine/internal/Engine.h"
#include "Engine/internal/Logging.h"
#include "Engine/include/Game.h"
#include "Engine/include/Logging.h"
#include "PArena.h"
#include "PMemory.h"
#include "PFileIO.h"
#include "PString.h"
#include "STD/internal/PMemory.h"
#include <Windows.h>

namespace {

	struct GameDll {
		pstd::DllHandle handle;
		Game::API api;
		bool isValid;
		size_t lastWriteTime;
	};

	pstd::String makeExeDirectoryPath(pstd::Arena* pPersistArena);

	GameDll loadGameDll(pstd::Arena scratchArena);
	void unloadGameDll(GameDll dll);

	peng::internal::State* engineState;
}  // namespace

int main() {
	Console::startup();

	pstd::AllocationRegistry allocationRegistry{ pstd::createAllocationRegistry(
	) };
	constexpr size_t scratchSize{ 1024 * 1024 };
	constexpr size_t gameSize{ 1024 * 1024 };

	pstd::Arena scratchArena{
		pstd::allocateArena(&allocationRegistry, scratchSize)
	};
	pstd::Arena secondaryScratchArena{
		pstd::allocateArena(&allocationRegistry, scratchSize)
	};

	pstd::Arena engineArena{ pstd::allocateArena(
		&allocationRegistry, peng::internal::getSizeofState()
	) };

	pstd::Arena gameArena{ pstd::allocateArena(&allocationRegistry, gameSize) };

	pstd::Arena runtimeArena{ pstd::allocateArena(&allocationRegistry, 1024) };

	engineState = peng::internal::startup(
		&allocationRegistry,
		&engineArena,
		pstd::makeLinked(scratchArena, &secondaryScratchArena)
	);

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
			gameDll = loadGameDll({ runtimeArena });
		}

		isRunning &= peng::internal::update(engineState);
		isRunning &= gameDll.api.update(gameState);
	}

	gameDll.api.shutdown(gameState);
	peng::internal::shutdown(engineState);
}

namespace {
	pstd::String makeExeDirectoryPath(pstd::Arena* pPersistArena) {
		pstd::String exeString{ pstd::getEXEPath(pPersistArena) };

		uint32_t seperatorIndex{};
		bool seperatorFound{ pstd::substringMatchBackward(
			exeString, pstd::createString("/"), &seperatorIndex
		) };
		if (!seperatorFound) {
			pstd::substringMatchBackward(
				exeString, pstd::createString("\\"), &seperatorIndex
			);
		}
		exeString.size = seperatorIndex + 1;
		return exeString;
	}

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
