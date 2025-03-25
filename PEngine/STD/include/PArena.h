#pragma once

#include "PTypes.h"
#include "PMemory.h"
#include "PAssert.h"
#include "PAlgorithm.h"

namespace pstd {
	struct Arena {
		Allocation allocation;
		uint32_t offset;
	};

	struct LinkedArenaPair {
		Arena current;
		const Arena* pNext;
	};

	template<typename T>
	constexpr size_t getCount(const Arena& arena) {
		return arena.offset / sizeof(T);
	}

	Arena allocateArena(AllocationRegistry* pAllocRegistry, size_t size);

	void freeArena(AllocationRegistry* pAllocRegistry, Arena* pArena);

	inline LinkedArenaPair
		makeLinked(const Arena& arenaSrc, const Arena* pArenaDst) {
		ASSERT(pArenaDst);

		LinkedArenaPair arenaPair{ .current = arenaSrc, .pNext = pArenaDst };
		arenaPair.current.allocation.ownsMemory = false;

		return arenaPair;
	}

	inline LinkedArenaPair getSwapped(const LinkedArenaPair* pArenaPair) {
		ASSERT(pArenaPair);
		ASSERT(pArenaPair->pNext);

		return makeLinked(*pArenaPair->pNext, &pArenaPair->current);
	}

	template<typename T>
	uint32_t getAvailableCount(const Arena& arena) {
		uintptr_t baseAddress{ (size_t)arena.allocation.block };
		uint32_t alignment{ alignof(T) };

		uint32_t alignmentPadding{
			(alignment -
			 vcast<uint32_t>((baseAddress + arena.offset) % alignment)) %
			alignment
		};

		uint32_t alignedOffset{ arena.offset + alignmentPadding };

		uint32_t availableBytes{ ncast<uint32_t>(arena.allocation.size) -
								 alignedOffset };
		return availableBytes / sizeof(T);
	}

	Allocation alloc(Arena* pArena, size_t size, uint32_t alignment);

	template<typename T>
	Allocation alloc(Arena* pArena, size_t count = 1) {
		ASSERT(pArena);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ alloc(pArena, allocSize, alignof(T)) };
	}

	inline Allocation makeShallowCopy(
		Arena* pArena, const Allocation& b, uint32_t alignment
	) {
		Allocation newAllocation{ pstd::alloc(pArena, b.size, alignment) };
		pstd::shallowCopy(&newAllocation, b);
		return newAllocation;
	}

	inline void reset(Arena* arena) {
		ASSERT(arena);

		arena->offset = 0;
	}
}  // namespace pstd
