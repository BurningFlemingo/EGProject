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

	pstd::String
		appendToExePath(pstd::ArenaFrame&& frame, const pstd::String& dllName);

	GameDll loadGameDll(pstd::ScratchArenaFrame&& frame);
	void unloadGameDll(GameDll dll);

	peng::internal::State* engineState;
}  // namespace

int main() {
	Console::startup();
	pstd::AllocationRegistry allocationRegistry{ pstd::createAllocationRegistry(
	) };
	size_t scratchSize{ 1024 * 1024 };
	size_t gameSize{ 1024 * 1024 };

	pstd::Arena engineArena{ pstd::allocateArena(
		&allocationRegistry, peng::internal::getSizeofState()
	) };

	pstd::Arena gameArena{
		pstd::allocateArena(&allocationRegistry, scratchSize + gameSize)
	};

	engineState =
		peng::internal::startup(&allocationRegistry, { &engineArena });

	// TODO: clean up code
	GameDll gameDll{ loadGameDll({ gameArena }) };
	Game::State* gameState{ gameDll.api.startup() };

	pstd::String dllPath{
		appendToExePath({ &gameArena }, pstd::createString("Game.dll"))
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
	pstd::String
		appendToExePath(pstd::ArenaFrame&& frame, const pstd::String& dllName) {
		pstd::String exeString{
			pstd::getEXEPath(pstd::makeFlipped({ frame.pArena, frame.state }))
		};

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
		pstd::String res{ pstd::makeConcatted(
			{ frame.pArena, frame.state }, exeString, dllName
		) };
		return res;
	}

	GameDll loadGameDll(pstd::ScratchArenaFrame&& scratchFrame) {
		pstd::ArenaFrame frame{
			.pArena = &scratchFrame.arena,
			.state = scratchFrame.state,
		};
		constexpr pstd::String writtenGameDllName{ pstd::createString("Game.dll"
		) };
		constexpr pstd::String runningGameDllName{
			pstd::createString("Game_Loaded.dll")
		};

		const char* writtenGameDllPath{ pstd::createCString(
			{ frame.pArena, frame.state },
			appendToExePath({ frame.pArena, frame.state }, writtenGameDllName)
		) };

		const char* runningGameDllPath{ pstd::createCString(
			{ frame.pArena, frame.state },
			appendToExePath({ frame.pArena, frame.state }, runningGameDllName)
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
