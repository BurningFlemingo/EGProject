#include "public/PConsole.h"
#include "public/PString.h"
#include "public/PAssert.h"

#include "private/PConsole.h"

#include <Windows.h>

namespace {
	HANDLE g_Stdout{};
}

void pcrt::initializeConsole() {
	g_Stdout = GetStdHandle(STD_OUTPUT_HANDLE);
}

bool pstd::consoleWrite(const pstd::String string) {
	if (g_Stdout != 0 && g_Stdout != INVALID_HANDLE_VALUE) {
		DWORD written{};
		return WriteConsoleA(
				   g_Stdout, string.buffer, string.size, &written, nullptr
			   ) != 0;
	}

	return 0;
}

bool pstd::consoleWrite(const char* cString) {
	pstd::String string{ pstd::createString(cString) };
	return consoleWrite(string);
}
