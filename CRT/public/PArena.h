#pragma once
#include <stdint.h>
#include <PMemory.h>

namespace pstd {
	struct FixedArena {
		Allocation allocation;
		size_t currentOffset;
		size_t savedOffset;
	};

	FixedArena
		allocateFixedArena(const size_t size, void* baseAddress = nullptr);

	template<typename T>
	FixedArena
		allocateFixedArena(const size_t count, void* baseAddress = nullptr) {
		const size_t bytesToAllocate{ count * sizeof(T) };
		allocateFixedArena(bytesToAllocate, baseAddress);
	}

	Allocation fixedAlloc(
		FixedArena* arena, const size_t size, const uint32_t alignment
	);

	template<typename T>
	Allocation fixedAlloc(FixedArena* arena, const size_t count = 1) {
		uint32_t alignment{ sizeof(T) };
		size_t allocSize{ count * alignment };
		Allocation allocation{ fixedAlloc(arena, allocSize, alignment) };
		return allocation;
	}

	void dealloc(FixedArena* arena);

	inline void saveArenaOffset(FixedArena* arena) {
		arena->savedOffset = arena->currentOffset;
	}

	inline void restoreArenaOffset(FixedArena* arena) {
		arena->currentOffset = arena->savedOffset;
	}

	inline void resetArena(FixedArena* arena) {
		arena->currentOffset = 0;
		arena->savedOffset = 0;
	}
}  // namespace pstd
