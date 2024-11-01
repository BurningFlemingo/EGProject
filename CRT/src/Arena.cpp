#include "public/PArena.h"
#include "public/PMemory.h"
#include "private/PMemory.h"
#include "public/PAssert.h"
#include <new>

pstd::FixedArena pstd::allocateFixedArena(const size_t size) {
	Allocation arenaAllocation{ internal::heapAlloc(size) };

	return FixedArena{ .allocation = arenaAllocation, .isAllocated = true };
}
void pstd::freeFixedArena(FixedArena* arena) {
	arena->isAllocated = false;
}

pstd::Allocation pstd::arenaAlloc(
	pstd::FixedArena* arena, const size_t size, const uint32_t alignment
) {
	ASSERT(arena->isAllocated);
	ASSERT(arena != nullptr);
	ASSERT(arena->allocation.block != nullptr);
	ASSERT(size != 0);
	ASSERT(alignment != 0);

	uint32_t alignmentPadding{ pstd::calcAddressAlignmentPadding(
		(void*)((size_t)arena->allocation.block + arena->offset), alignment
	) };

	ASSERT((arena->offset + size + alignmentPadding) <= arena->allocation.size);

	size_t alignedHeadAddress{
		((size_t)arena->allocation.block + arena->offset + alignmentPadding)
	};
	arena->offset += size + alignmentPadding;

	return pstd::Allocation{ .block = (void*)alignedHeadAddress, .size = size };
}
