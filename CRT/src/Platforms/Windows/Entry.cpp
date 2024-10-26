#include <Windows.h>
#include "PTypes.h"

#include "private/PMemory.h"
#include "private/PConsole.h"

int main();

extern "C" int mainCRTStartup() {
	pstd::initializeMemorySystem();
	pstd::initializeConsole();

	int exitCode{ main() };

	ExitProcess(exitCode);

	return 0;
}
extern "C" int WinMainCRTStartup() {
	return 0;
}
