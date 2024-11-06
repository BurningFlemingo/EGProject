#pragma once
#include "STD/include/PArena.h"

namespace pstd {
	namespace internal {
		FixedArena allocateFixedArena(const size_t size);
		template<typename T>
		FixedArena allocateFixedArena(const size_t count) {
			size_t byteAllocSize{ count * sizeof(T) };
			return FixedArena{ allocateFixedArena(byteAllocSize) };
		}
	}  // namespace internal
}  // namespace pstd
