#pragma once
#include "STD/include/PArena.h"
#include "STD/internal/PMemory.h"

namespace peng {
	namespace internal {
		struct State;

		size_t getSizeofState();

		State* startup(
			pstd::internal::AllocationRegistry* registry,
			pstd::FixedArena* stateArena
		);
		bool update(State* state);
		void shutdown(State* state);
	}  // namespace internal
}  // namespace peng
