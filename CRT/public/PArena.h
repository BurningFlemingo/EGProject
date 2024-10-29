#pragma once
#include "PTypes.h"
#include "PMemory.h"
#include "PAssert.h"

namespace pstd {
	struct FixedArena {
		Allocation allocation;
		size_t offset;
	};

	template<typename T>
	constexpr size_t getCount(const FixedArena* arena) {
		size_t res{ arena->offset };
		res /= sizeof(T);
		return res;
	}
	template<typename T>
	constexpr size_t getAvaliableCount(const FixedArena& arena) {
		size_t allocPadding{ pstd::calcAddressAlignmentPadding<T>(
			(void*)((size_t)arena.allocation.block + arena.offset)
		) };
		size_t offset{ arena.offset + allocPadding };

		size_t res{};
		if (arena.allocation.size < offset) {
			return res;
		}
		res = (arena.allocation.size - offset) / sizeof(T);
		return res;
	}

	FixedArena allocateFixedArena(const size_t size);
	template<typename T>
	FixedArena allocateFixedArena(const size_t count) {
		size_t byteAllocSize{ count * sizeof(T) };
		FixedArena arena{ allocateFixedArena(byteAllocSize) };
		return arena;
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
		Allocation allocation{ arenaAlloc(arena, allocSize, alignment) };
		return allocation;
	}

	template<typename T>
	void* getNextAllocAddress(const FixedArena& arena) {
		size_t headAddress{ (size_t)arena.allocation.block + arena.offset };
		size_t padding{ pstd::calcAddressAlignmentPadding<T>((void*)headAddress
		) };
		size_t res{ headAddress + padding };
		return (void*)res;
	}

	constexpr inline void reset(FixedArena* arena) {
		ASSERT(arena);

		arena->offset = 0;
	}
}  // namespace pstd
