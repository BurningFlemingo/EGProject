#include "Logging.h"
#include "LoggingSetup.h"

#include "STD/PTypes.h"
#include "STD/PArena.h"
#include "STD/PConsole.h"
#include "STD/PString.h"
#include "STD/PAssert.h"

namespace {
	constexpr uint32_t LOG_ARENA_SIZE{ 1024 * 1024 };
	char g_RawLogArray[LOG_ARENA_SIZE]{};
	pstd::Arena g_LogArena{};

	const char* g_LogLevelStrings[4]{ "", "[INFO]", "[WARNING]", "[ERROR]" };

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

	pstd::String logLevelString{
		pstd::createString(g_LogLevelStrings[ncast<size_t>(logLevel)])
	};

	pstd::consoleWrite(logLevelString);
	pstd::consoleWrite(msg);

	pstd::reset(&g_LogArena);
}
