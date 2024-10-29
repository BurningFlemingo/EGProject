#include "public/PConsole.h"
#include "public/PString.h"
#include "public/PAssert.h"

#include "private/PConsole.h"

#include <Windows.h>

namespace {
	HANDLE g_Stdout{};
}

void pstd::internal::initializeConsole() {
	g_Stdout = GetStdHandle(STD_OUTPUT_HANDLE);
}

bool pstd::consoleWrite(const pstd::String string) {
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
