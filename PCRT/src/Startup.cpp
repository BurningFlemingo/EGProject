#include <Windows.h>
#include "private/PMemory.h"
#include "private/PConsole.h"

extern "C" int DllMain(
	const HINSTANCE instance, const DWORD reason, const LPVOID reserved
) {
	if (reason == DLL_PROCESS_DETACH) {
		return FALSE;
	}
	if (reason == DLL_PROCESS_ATTACH) {
		pcrt::initializeMemorySystem();
		pcrt::initializeConsole();
		return TRUE;
	}
	return TRUE;
}
