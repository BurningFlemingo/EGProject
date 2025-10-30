#pragma once
#include "Core/PArena.h"
#include "Core/PMemory.h"

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
