#include "Engine/internal/Engine.h"
#include "PArena.h"
#include "STD/internal/PArena.h"
#include "STD/internal/PMemory.h"

int main() {
	pstd::internal::AllocationRegistry allocationRegistry{
		pstd::internal::startupHeap()
	};
	pstd::FixedArena arena{ pstd::internal::allocateFixedArena(
		&allocationRegistry, peng::internal::getSizeofState()
	) };

	peng::internal::State* engineState{
		peng::internal::startup(&allocationRegistry, &arena)
	};
	bool isRunning{ true };
	while (isRunning) {
		isRunning = peng::internal::update(engineState);
	}

	peng::internal::shutdown(engineState);
}
