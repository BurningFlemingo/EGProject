#pragma once
#include "STD/PArena.h"
#include "STD/PMemory.h"

namespace PE {

	struct State;

	size_t getSizeofState();

	State* startup(
		pstd::Arena* pPersistArena,
		pstd::Arena scratchArena,
		pstd::AllocationRegistry* pRegistry
	);
	bool update(State* state);
	void shutdown(State* state);

}  // namespace PE
