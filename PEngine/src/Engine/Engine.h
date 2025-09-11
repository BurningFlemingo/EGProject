#pragma once
#include "Core/PArena.h"
#include "Core/PMemory.h"

namespace PE {

	struct State;

	size_t getSizeofState();

	State* startup(
		pstd::AllocationRegistry* registry,
		pstd::Arena* pPersistArena,
		pstd::ArenaPair scratchArenas
	);
	bool update(State* state);
	void shutdown(State* state);

}  // namespace PE
