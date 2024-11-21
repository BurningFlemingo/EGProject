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

	pstd::String makeExeDirectoryPath(pstd::ArenaFrame&& frame);

	GameDll loadGameDll(pstd::ArenaScratchFrame&& frame);
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

	pstd::Arena runtimeArena{
		pstd::allocateArena(&allocationRegistry, 1024 + scratchSize)
	};

	engineState = peng::internal::startup(
		&allocationRegistry, pstd::makeFrame(&engineArena)
	);

	GameDll gameDll{ loadGameDll(pstd::makeScratchFrame(gameArena)) };
	Game::State* gameState{ gameDll.api.startup() };

	pstd::String originalDllPath{ pstd::formatString(
		pstd::makeFrame(&runtimeArena),
		"%mGame.%m",
		makeExeDirectoryPath({ &runtimeArena }),
		pstd::getDllExtensionName()
	) };

	bool isRunning{ true };
	const char* originalDllPathCString{
		pstd::createCString(pstd::makeFrame(&runtimeArena), originalDllPath)
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
	pstd::String makeExeDirectoryPath(pstd::ArenaFrame&& frame) {
		pstd::String exeString{
			pstd::getEXEPath(pstd::makeFrame(frame, &frame.scratchOffset))
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
		return exeString;
	}

	GameDll loadGameDll(pstd::ArenaScratchFrame&& frame) {
		static uint32_t loadedDllSlot{};

		constexpr pstd::String originalDllName{ pstd::createString("Game") };

		pstd::String loadedDllPath{ pstd::formatString(
			pstd::makeFrame(&frame),
			"%mGame_Loaded_%u.%m",
			makeExeDirectoryPath(pstd::makeFrame(&frame)),
			loadedDllSlot,
			pstd::getDllExtensionName()
		) };

		uint32_t unloadedDllSlot{ (loadedDllSlot + 1) % 2 };

		pstd::String toLoadDllPath{ pstd::formatString(
			pstd::makeFrame(&frame),
			"%mGame_Loaded_%u.%m",
			makeExeDirectoryPath(pstd::makeFrame(&frame)),
			unloadedDllSlot,
			pstd::getDllExtensionName()
		) };

		pstd::String originalDllPath{ pstd::formatString(
			pstd::makeFrame(&frame),
			"%mGame.%m",
			makeExeDirectoryPath(pstd::makeFrame(&frame)),
			pstd::getDllExtensionName()
		) };

		pstd::copyFile(
			pstd::createCString(pstd::makeFrame(&frame), toLoadDllPath),
			pstd::createCString(pstd::makeFrame(&frame), originalDllPath),
			true
		);

		pstd::DllHandle gameHandle{ pstd::loadDll(
			pstd::createCString(pstd::makeFrame(&frame), toLoadDllPath)
		) };
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
