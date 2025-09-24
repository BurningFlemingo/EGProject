#include "Logging.h"
#include "LoggingSetup.h"

#include "Core/PTypes.h"
#include "Core/PArena.h"
#include "Core/PConsole.h"
#include "Core/PString.h"
#include "Core/PAssert.h"

namespace {
	constexpr uint32_t LOG_ARENA_SIZE{ 1024 * 1024 };
	char g_RawLogArray[LOG_ARENA_SIZE]{};
	pstd::Arena g_LogArena{};

	const char* g_LogLevelStringsBuffer[4]{
		"", "[INFO]: ", "[WARNING] ", "[ERROR] "
	};

	constexpr pstd::Array<const char*, Console::LogLevel> g_LogLevelStrings{
		pstd::createArray<const char*, Console::LogLevel>(
			g_LogLevelStringsBuffer
		)
	};
}  // namespace

void Console::startup() {
	g_LogArena = pstd::Arena{ .block = rcast<uint8_t*>(g_RawLogArray),
							  .size = LOG_ARENA_SIZE };
}

pstd::Arena Console::getLogArena() {
	if (g_LogArena.block == nullptr) {
		Console::startup();
	}
	return g_LogArena;
}

void Console::log(const Console::LogLevel logLevel, const pstd::String& msg) {
	ASSERT(logLevel <= LogLevel::error);
	if (g_LogArena.block == nullptr) {
		Console::startup();
	}

	pstd::reset(&g_LogArena);

	pstd::String logLevelString{ pstd::createString(g_LogLevelStrings[logLevel]
	) };
	pstd::consoleWrite(logLevelString);
	pstd::consoleWrite(msg);

	pstd::reset(&g_LogArena);
}
