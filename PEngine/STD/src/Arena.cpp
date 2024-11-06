#include "include/PArena.h"
#include "internal/PArena.h"
#include "include/PMemory.h"
#include "internal/PMemory.h"
#include "include/PAssert.h"
#include <new>

using namespace pstd;
using namespace pstd::internal;

pstd::FixedArena pstd::internal::allocateFixedArena(const size_t size) {
	Allocation arenaAllocation{ heapAlloc(size) };

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

	return Allocation{ .block = (void*)alignedHeadAddress, .size = size };
}
