#include "Logging.h"
#include "PArena.h"
#include "PConsole.h"
#include "PString.h"
#include "PAssert.h"

namespace {
	constexpr size_t LOG_ARENA_SIZE{ 1024 };
	char g_RawLogArray[LOG_ARENA_SIZE]{};
	pstd::FixedArena g_LogArena{ .allocation = { .block = (void*)g_RawLogArray,
												 .size = LOG_ARENA_SIZE } };

	constexpr const char* g_LogLevelStrings[(uint32_t)Console::LogLevel::count]{
		"", "[INFO]: ", "[WARNING] ", "[ERROR] "
	};
}  // namespace

pstd::FixedArena Console::getLogArena() {
	return g_LogArena;
}

void Console::log(const Console::LogLevel logLevel, const pstd::String& msg) {
	ASSERT(logLevel <= (uint32_t)LogLevel::error);

	pstd::reset(&g_LogArena);

	pstd::String logLevelString{
		pstd::createString(g_LogLevelStrings[(uint32_t)logLevel])
	};
	pstd::consoleWrite(logLevelString);
	pstd::consoleWrite(msg);

	pstd::reset(&g_LogArena);
}
