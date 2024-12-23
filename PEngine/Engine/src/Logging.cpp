#include "include/Logging.h"
#include "internal/Logging.h"
#include "PArena.h"
#include "PConsole.h"
#include "PString.h"
#include "PAssert.h"

namespace {
	constexpr uint32_t LOG_ARENA_SIZE{ 1024 };
	char g_RawLogArray[LOG_ARENA_SIZE]{};
	pstd::Arena g_LogArena{};

	constexpr pstd::StaticArray<
		const char*,
		cast<size_t>(Console::LogLevel::count),
		Console::LogLevel>
		g_LogLevelStrings{ .data = {
							   "", "[INFO]: ", "[WARNING] ", "[ERROR] " } };
}  // namespace

void Console::startup() {
	g_LogArena =
		pstd::Arena{ .allocation = { .block = rcast<uint8_t*>(g_RawLogArray),
									 .size = LOG_ARENA_SIZE },
					 .isAllocated = true };
}

pstd::Arena Console::getLogArena() {
	if (!g_LogArena.isAllocated || g_LogArena.allocation.block == nullptr) {
		Console::startup();
	}
	return g_LogArena;
}

void Console::log(const Console::LogLevel logLevel, const pstd::String& msg) {
	ASSERT(logLevel <= LogLevel::error);
	if (!g_LogArena.isAllocated || g_LogArena.allocation.block == nullptr) {
		Console::startup();
	}

	pstd::reset(&g_LogArena);

	pstd::String logLevelString{ pstd::createString(g_LogLevelStrings[logLevel]
	) };
	pstd::consoleWrite(logLevelString);
	pstd::consoleWrite(msg);

	pstd::reset(&g_LogArena);
}
