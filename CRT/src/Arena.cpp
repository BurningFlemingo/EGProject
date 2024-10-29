#include "public/PArena.h"
#include "public/PMemory.h"
#include "private/PMemory.h"
#include "public/PAssert.h"

pstd::FixedArena pstd::allocateFixedArena(const size_t size) {
	const pstd::Allocation allocation{ pstd::internal::heapAlloc(size) };
	FixedArena arena{ .allocation = allocation };
	return arena;
}
void pstd::freeFixedArena(FixedArena* arena) {
	pstd::internal::heapFree(&arena->allocation);
}

pstd::Allocation pstd::arenaAlloc(
	pstd::FixedArena* arena, const size_t size, const uint32_t alignment
) {
	ASSERT(arena != nullptr);
	ASSERT(arena->allocation.block != nullptr);
	ASSERT(size != 0);
	ASSERT(alignment != 0);

	uint32_t alignmentPadding{
		pstd::calcAddressAlignmentPadding(arena->allocation.block, alignment)
	};

	ASSERT((arena->offset + size + alignmentPadding) <= arena->allocation.size);

	size_t initialHeadOffset{ arena->offset };

	size_t alignedHeadAddress{
		((size_t)arena->allocation.block + arena->offset + alignmentPadding)
	};
	arena->offset += size + alignmentPadding;

	pstd::Allocation allocation{ .block = (void*)alignedHeadAddress,
								 .size = size };
	return allocation;
}
