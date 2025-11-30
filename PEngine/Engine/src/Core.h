#pragma once
#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "Engine.h"

namespace Engine {

	Subsystems startup(pstd::AllocationRegistry* pAllocRegistry);
	bool update(
		pstd::AllocationRegistry* pAllocRegistry, const Subsystems& state
	);
	// returns isRunning
	bool
		tick(pstd::AllocationRegistry* pAllocRegistry, const Subsystems& state);
	void shutdown(const Subsystems& state);

}  // namespace Engine
