#pragma once
#include <stdint.h>
#include "PMemory.h"
#include "PAssert.h"

namespace pstd {
	struct FixedArena {
		Allocation allocation;
		size_t offset;
	};

	Allocation bufferAlloc(
		FixedArena* arena, const size_t size, const uint32_t alignment
	);

	template<typename T>
	Allocation bufferAlloc(FixedArena* arena, const size_t count = 1) {
		ASSERT(arena);

		uint32_t alignment{ sizeof(T) };
		size_t allocSize{ count * alignment };
		Allocation allocation{ bufferAlloc(arena, allocSize, alignment) };
		return allocation;
	}

	constexpr inline void resetArena(FixedArena* arena) {
		ASSERT(arena);

		arena->offset = 0;
	}
}  // namespace pstd
