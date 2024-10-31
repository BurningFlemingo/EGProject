#pragma once
#include "PTypes.h"
#include "PMemory.h"
#include "PAssert.h"

namespace pstd {
	struct FixedArena {
		const Allocation allocation;
		size_t offset;

		bool isAllocated;
	};

	template<typename T>
	constexpr size_t getCount(const FixedArena* arena) {
		return arena->offset / sizeof(T);
	}
	template<typename T>
	constexpr size_t getAvaliableCount(const FixedArena& arena) {
		size_t allocPadding{ pstd::calcAddressAlignmentPadding<T>(
			(void*)((size_t)arena.allocation.block + arena.offset)
		) };
		size_t offset{ arena.offset + allocPadding };

		if (arena.allocation.size <= offset) {
			return 0;
		}
		size_t avaliableBytes{ arena.allocation.size - offset };
		size_t res{ avaliableBytes / sizeof(T) };
		return res;
	}

	FixedArena allocateFixedArena(const size_t size);
	template<typename T>
	FixedArena allocateFixedArena(const size_t count) {
		size_t byteAllocSize{ count * sizeof(T) };
		return FixedArena{ allocateFixedArena(byteAllocSize) };
	}
	void freeFixedArena(FixedArena* arena);

	Allocation arenaAlloc(
		FixedArena* arena, const size_t size, const uint32_t alignment
	);

	template<typename T>
	Allocation arenaAlloc(FixedArena* arena, const size_t count = 1) {
		ASSERT(arena);

		uint32_t alignment{ sizeof(T) };
		size_t allocSize{ count * alignment };
		return Allocation{ arenaAlloc(arena, allocSize, alignment) };
	}

	template<typename T>
	void* getNextAllocAddress(const FixedArena& arena) {
		size_t headAddress{ (size_t)arena.allocation.block + arena.offset };
		size_t padding{ pstd::calcAddressAlignmentPadding<T>((void*)headAddress
		) };
		void* res{ (void*)(headAddress + padding) };
		return res;
	}

	constexpr inline void reset(FixedArena* arena) {
		ASSERT(arena);

		arena->offset = 0;
	}
}  // namespace pstd
