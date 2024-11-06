#include "Engine/internal/Engine.h"
#include "PArena.h"
#include "STD/internal/PArena.h"

int main() {
	pstd::FixedArena arena{
		pstd::internal::allocateFixedArena(peng::internal::getSizeofState())
	};

	peng::internal::State* engineState{ peng::internal::startup(&arena) };
	bool isRunning{ true };
	while (isRunning) {
		isRunning = peng::internal::update(engineState);
	}

	peng::internal::shutdown(engineState);
}
