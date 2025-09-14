#pragma once

#include "PTypes.h"
#include "PMemory.h"
#include "PAssert.h"
#include "PAlgorithm.h"

namespace pstd {
	struct Arena {
		void* block;
		size_t size;
		uint32_t offset;
	};

	struct ArenaPair {
		Arena first;
		Arena second;
	};

	inline Arena* getUnique(ArenaPair* pArenas, Arena* pArena) {
		ASSERT(pArenas);
		ASSERT(pArena);

		if (pArena == &pArenas->first) {
			return &pArenas->second;
		}
		return &pArenas->first;
	}

	template<typename T>
	constexpr size_t getCount(const Arena& arena) {
		return arena.offset / sizeof(T);
	}

	inline bool isAliasing(const Arena& a, const Arena& b) {
		return isAliasing(a.block, b.block);
	}

	Arena allocateArena(AllocationRegistry* pAllocRegistry, size_t size);

	void freeArena(AllocationRegistry* pAllocRegistry, Arena* pArena);

	template<typename T>
	uint32_t getAvailableCount(const Arena& arena) {
		// TODO: change this for cast
		uintptr_t baseAddress{ vcast<uintptr_t>(arena.block) };
		uint32_t alignment{ alignof(T) };

		uint32_t alignmentPadding{
			(alignment -
			 vcast<uint32_t>((baseAddress + arena.offset) % alignment)) %
			alignment
		};

		uint32_t alignedOffset{ arena.offset + alignmentPadding };

		uint32_t availableBytes{ ncast<uint32_t>(arena.size) - alignedOffset };
		return availableBytes / sizeof(T);
	}

	void* alloc(Arena* pArena, size_t size, uint32_t alignment);

	template<typename T>
	T* alloc(Arena* pArena, size_t count = 1) {
		ASSERT(pArena);

		size_t allocSize{ count * sizeof(T) };
		return rcast<T*>(alloc(pArena, allocSize, alignof(T)));
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
