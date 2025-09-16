#include "Core/PConsole.h"
#include "Core/PString.h"
#include "Core/PAssert.h"

#include "Core/Console.h"

#include <Windows.h>

namespace {
	HANDLE g_Stdout{};
}

void pstd::startupConsole() {
	g_Stdout = GetStdHandle(STD_OUTPUT_HANDLE);
}

bool pstd::consoleWrite(const pstd::String string) {
	if (!g_Stdout) {
		g_Stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	if (g_Stdout != INVALID_HANDLE_VALUE) {
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
