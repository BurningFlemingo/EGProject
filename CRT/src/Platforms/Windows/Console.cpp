#include "PConsole.h"
#include "PString.h"

#include <Windows.h>

namespace {
	HANDLE g_Stdout{};
}

bool pstd::consoleWrite(const pstd::String string) {
	if (g_Stdout == 0) {
		g_Stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	bool res{};
	if (g_Stdout != 0 && g_Stdout != INVALID_HANDLE_VALUE) {
		DWORD written{};
		res = WriteConsoleA(
			g_Stdout, string.buffer, string.size, &written, nullptr
		);
	}

	return res;
}

bool pstd::consoleWrite(const char* cString) {
	pstd::String string{ pstd::createString(cString) };
	return consoleWrite(string);
}
