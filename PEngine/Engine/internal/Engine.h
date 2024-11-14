#pragma once
#include "STD/include/PArena.h"
#include "STD/include/PMemory.h"

namespace peng {
	namespace internal {
		struct State;

		size_t getSizeofState();

		State* startup(
			pstd::AllocationRegistry* registry, pstd::ArenaFrame&& arenaFrame
		);
		bool update(State* state);
		void shutdown(State* state);
	}  // namespace internal
}  // namespace peng
