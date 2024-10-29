#include "public/PArena.h"
#include "public/PMemory.h"
#include "private/PMemory.h"
#include "public/PAssert.h"

using namespace _pstd;
using namespace pstd;

FixedArena pstd::allocateFixedArena(const size_t size) {
	const pstd::Allocation allocation{ heapAlloc(size) };
	FixedArena arena{ .allocation = allocation };
	return arena;
}
void pstd::freeFixedArena(FixedArena* arena) {
	heapFree(&arena->allocation);
}

pstd::Allocation pstd::bufferAlloc(
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
