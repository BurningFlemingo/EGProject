#include <Windows.h>
#include <stdint.h>

#include "private/PMemory.h"

int main();

extern "C" int mainCRTStartup() {
	pstd::initializeMemorySystem();

	int exitCode{ main() };

	pstd::cleanupMemorySystem();

	ExitProcess(exitCode);

	return 0;
}
extern "C" int WinMainCRTStartup() {
	return 0;
}
