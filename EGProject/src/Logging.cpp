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

template<typename T>
void Console::log(
	const pstd::String& format, const T val, const Console::LogLevel logLevel
) {
	ASSERT(logLevel <= (uint32_t)LogLevel::error);

	pstd::reset(&g_LogArena);

	pstd::String logLevelString{
		pstd::createString(g_LogLevelStrings[(uint32_t)logLevel])
	};
	pstd::String msg{ pstd::formatString(&g_LogArena, format, val) };
	pstd::consoleWrite(logLevelString);
	pstd::consoleWrite(msg);

	pstd::reset(&g_LogArena);
}

void Console::log(const pstd::String& msg, const Console::LogLevel logLevel) {
	ASSERT(logLevel <= (uint32_t)LogLevel::error);

	pstd::reset(&g_LogArena);

	pstd::String logLevelString{
		pstd::createString(g_LogLevelStrings[(uint32_t)logLevel])
	};
	pstd::consoleWrite(logLevelString);
	pstd::consoleWrite(msg);

	pstd::reset(&g_LogArena);
}

void Console::log(const char* msg, const Console::LogLevel logLevel) {
	pstd::String msgString{ pstd::createString(msg) };
	log(msgString, logLevel);
}
template void Console::log(
	const pstd::String& format,
	const float val,
	const Console::LogLevel logLevel
);

template void Console::log(
	const pstd::String& format,
	const uint32_t val,
	const Console::LogLevel logLevel
);

template void Console::log(
	const pstd::String& format,
	const int32_t val,
	const Console::LogLevel logLevel
);
