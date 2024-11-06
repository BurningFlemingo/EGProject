#include "Engine/internal/Engine.h"
#include "Engine/include/Game.h"
#include "Engine/include/Logging.h"
#include "PArena.h"
#include "PMemory.h"
#include "PFileIO.h"
#include "PString.h"
#include "STD/internal/PMemory.h"

namespace {

	struct GameDll {
		Game::API api;
		bool isValid;
		size_t lastWriteTime;
	};

	pstd::String
		createDllString(pstd::FixedArena* scratchArena, const char* dllName);

	GameDll loadGameDll(pstd::FixedArena* scratchArena);
	void unloadGameDll(GameDll dll);
}  // namespace

int main() {
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

	Game::State* gameState{ gameAPI.startup() };

	bool isRunning{ true };
	while (isRunning) {
		isRunning = peng::internal::update(engineState);
	}

	gameAPI.shutdown(gameState);
	peng::internal::shutdown(engineState);
}

namespace {
	pstd::String
		createDLLString(pstd::FixedArena* scratchArena, const char* dllName) {
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
		exeString.size = seperatorIndex;
		pstd::String res{
			pstd::concat(scratchArena, exeString, pstd::createString("/Game"))
		};
		return res;
	}

	GameDll loadGameDll(pstd::FixedArena* scratchArena) {
		// TODO: move to platform layer, this is only for testing.
		const char* dllCString{ "Game.dll" };
		const char* dllIndermediateCString{ "Game_Loaded.dll" };
		pstd::copyFile(dllCString, dllIndermediateCString, true);

		const char* dllString{ pstd::createCString(
			scratchArena, createDllString(scratchArena, dllIndermediateCString)
		) };

		pstd::DllHandle gameHandle{ pstd::loadDll(dllString) };

		Game::API gameAPI{
			.startup = (Game::API::Startup
			)pstd::findDllFunction(gameHandle, "startup"),
			.update =
				(Game::API::Update)pstd::findDllFunction(gameHandle, "update"),
			.shutdown = (Game::API::Shutdown
			)pstd::findDllFunction(gameHandle, "shutdown"),
		};
		bool isValid{ gameAPI.shutdown && gameAPI.update && gameAPI.shutdown };

		GameDll res{ .api = gameAPI,
					 .isValid = isValid,
					 .lastWriteTime = pstd::getLastFileWriteTime(dllCString) };
		return res;
	}
	void unloadGameDll(GameDll dll);
}  // namespace
