#include "Core/PAssert.h"
#include <Windows.h>

namespace {
	const char* getFileName(const char* filePath);
}

bool pstd::consoleWriteAssertionFailure(
	const char* file, int line, const char* expr, const char* msg
) {
	if (msg == nullptr) {
		msg = "none";
	}

	HANDLE stderr{ GetStdHandle(STD_ERROR_HANDLE) };

	char buffer[512];
	int bufSize{ wsprintfA(
		buffer,
		"[ASSERTION FAILURE] [%s:%i]\n"
		"-----------------------------------------\n"
		"Expression: %s\n"
		"Message: %s\n"
		"-----------------------------------------\n",
		getFileName(file),
		line,
		expr,
		msg
	) };

	if (stderr != INVALID_HANDLE_VALUE) {
		DWORD written{};
		return WriteFile(stderr, buffer, bufSize, &written, nullptr) != 0;
	}

	return 0;
}

namespace {
	const char* getFileName(const char* name) {
		for (const char* path{ name }; *path != '\0'; path++) {
			if (*path == '\\' || *path == '/') {
				name = path + 1;
			}
		}

		return name;
	}
}  // namespace
