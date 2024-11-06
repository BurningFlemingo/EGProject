#include "Engine/internal/Engine.h"
#include "PArena.h"
#include "PMemory.h"
#include "STD/internal/PMemory.h"

int main() {
	pstd::AllocationRegistry allocationRegistry{ pstd::createAllocationRegistry(
	) };
	pstd::FixedArena arena{ pstd::allocateFixedArena(
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
