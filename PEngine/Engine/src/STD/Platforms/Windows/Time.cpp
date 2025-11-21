#include "STD/PTime.h"
#include <Windows.h>
#include "STD/PTypes.h"

namespace {
	LARGE_INTEGER g_InitialTicks{};
}

double pstd::getTicks() {
	LARGE_INTEGER ticks{};
	QueryPerformanceCounter(&ticks);

	if (g_InitialTicks.QuadPart == 0) {
		g_InitialTicks = ticks;

		return 0;
	}

	LARGE_INTEGER freq{};
	QueryPerformanceFrequency(&freq);

	return ncast<double>(
		((ticks.QuadPart - g_InitialTicks.QuadPart) * 1000) / (freq.QuadPart)
	);
}
