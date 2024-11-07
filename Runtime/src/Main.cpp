#include "Engine/internal/Engine.h"
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

	pstd::String appendToExePath(
		pstd::FixedArena* scratchArena, const pstd::String& dllName
	);

	GameDll loadGameDll(pstd::FixedArena arena);
	void unloadGameDll(GameDll dll);
}  // namespace

int main() {
	// TODO: clean up code
	pstd::AllocationRegistry allocationRegistry{ pstd::createAllocationRegistry(
	) };
	pstd::FixedArena engineArena{ pstd::allocateFixedArena(
		&allocationRegistry, peng::internal::getSizeofState()
	) };
	pstd::FixedArena scratchArena{
		pstd::allocateFixedArena(&allocationRegistry, 1024 * 1024)
	};

	peng::internal::State* engineState{
		peng::internal::startup(&allocationRegistry, &engineArena)
	};

	GameDll gameDll{ loadGameDll(scratchArena) };
	Game::State* gameState{ gameDll.api.startup() };

	pstd::String dllPath{
		appendToExePath(&scratchArena, pstd::createString("Game.dll"))
	};

	bool isRunning{ true };
	while (isRunning) {
		// if (pstd::getLastFileWriteTime(
		// 		pstd::createCString(&scratchArena, dllPath)
		// 	) != gameDll.lastWriteTime) {
		// 	unloadGameDll(gameDll);
		// 	gameDll = loadGameDll(scratchArena);
		// }

		isRunning &= peng::internal::update(engineState);
		isRunning &= gameDll.api.update(gameState);
	}

	gameDll.api.shutdown(gameState);
	peng::internal::shutdown(engineState);
}

namespace {
	pstd::String appendToExePath(
		pstd::FixedArena* scratchArena, const pstd::String& dllName
	) {
		pstd::String exeString{ pstd::getEXEPath(scratchArena) };
		size_t seperatorIndex{};
		bool seperatorFound{ pstd::substringMatchBackward(
			exeString, pstd::createString("/"), &seperatorIndex
		) };
		if (!seperatorFound) {
			pstd::substringMatchBackward(
				exeString, pstd::createString("\\"), &seperatorIndex
			);
		}
		exeString.size = seperatorIndex + 1;
		pstd::String res{ pstd::concat(scratchArena, exeString, dllName) };
		return res;
	}

	GameDll loadGameDll(pstd::FixedArena arena) {
		constexpr pstd::String writtenGameDllName{ pstd::createString("Game.dll"
		) };
		constexpr pstd::String runningGameDllName{
			pstd::createString("Game_Loaded.dll")
		};

		const char* writtenGameDllPath{ pstd::createCString(
			&arena, appendToExePath(&arena, writtenGameDllName)
		) };

		const char* runningGameDllPath{ pstd::createCString(
			&arena, appendToExePath(&arena, runningGameDllName)
		) };

		pstd::copyFile(
			writtenGameDllPath, runningGameDllPath, true
		);	// running game dll should be unloaded at this point

		pstd::DllHandle gameHandle{ pstd::loadDll(runningGameDllPath) };

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
						 pstd::getLastFileWriteTime(writtenGameDllPath) };
		return res;
	}
	void unloadGameDll(GameDll dll) {
		if (dll.handle) {
			pstd::unloadDll(dll.handle);
		}
	}
}  // namespace
