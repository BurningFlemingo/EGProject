#include <Windows.h>
#include <stdint.h>

int main();

extern "C" int mainCRTStartup() {
	int exitCode{ main() };

	ExitProcess(exitCode);

	return 0;
}
extern "C" int WinMainCRTStartup() {
	return 0;
}
