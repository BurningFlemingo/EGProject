#pragma once
#include <stdint.h>
#include <PMemory.h>

namespace pstd {
	struct FixedArena {
		Allocation<void> allocation;
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

	Allocation<void> fixedArenaAlloc(
		FixedArena* arena, const size_t size, const uint32_t alignment
	);

	template<typename T>
	Allocation<T> fixedArenaAlloc(FixedArena* arena, const size_t count = 1) {
		uint32_t alignment{ sizeof(T) };
		Allocation voidAllocation{ fixedArenaAlloc(arena, count, alignment) };
		Allocation<T> allocation{ .block = (T*)voidAllocation.block,
								  .size = voidAllocation.size };
		return allocation;
	}

	void destroyFixedArena(FixedArena* arena);

	inline void saveArenaOffset(FixedArena* arena) {
		arena->savedOffset = arena->currentOffset;
	}

	inline void restoreArenaOffset(FixedArena* arena) {
		arena->currentOffset = arena->savedOffset;
	}

	void resetArena(FixedArena* arena);
}  // namespace pstd
