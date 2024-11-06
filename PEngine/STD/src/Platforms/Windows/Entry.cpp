#include "include/PTypes.h"
#include "internal/PMemory.h"
#include "internal/PConsole.h"

#include <Windows.h>

extern "C" int main();
extern "C" int stubMain() {
	return {};
}

extern "C" BOOL WINAPI DllMain(
	const HINSTANCE instance, const DWORD reason, const LPVOID reserved
);

extern "C" BOOL WINAPI stubDllMain(
	const HINSTANCE instance, const DWORD reason, const LPVOID reserved
) {
	DisableThreadLibraryCalls(instance);
	return TRUE;
}

#pragma comment(linker, "/alternatename:main=stubMain")
#pragma comment(linker, "/alternatename:DllMain=stubDllMain")

extern "C" int mainCRTStartup() {
	pstd::internal::startupMemory();
	pstd::internal::startupConsole();

	int exitCode{ main() };

	ExitProcess(exitCode);

	return 0;
}
extern "C" int WinMainCRTStartup() {
	return 0;
}

extern "C" int _DllMainCRTStartup(
	const HINSTANCE instance, const DWORD reason, const LPVOID reserved
) {
	if (reason == DLL_PROCESS_DETACH) {
		return FALSE;
	}
	if (reason == DLL_PROCESS_ATTACH) {
		BOOL exitCode{ DllMain(instance, reason, reserved) };
		return exitCode;
	}
	return TRUE;
}
