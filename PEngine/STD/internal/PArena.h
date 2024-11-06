#pragma once
#include "STD/include/PArena.h"
#include "PMemory.h"

namespace pstd {
	namespace internal {
		FixedArena
			allocateFixedArena(AllocationRegistry* registry, const size_t size);
		template<typename T>
		FixedArena allocateFixedArena(
			AllocationRegistry* registry, const size_t count
		) {
			size_t byteAllocSize{ count * sizeof(T) };
			return FixedArena{ allocateFixedArena(registry, byteAllocSize) };
		}
	}  // namespace internal
}  // namespace pstd
