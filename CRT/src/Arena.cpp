#include "PArena.h"
#include "PMemory.h"
#include "PAssert.h"

pstd::FixedArena
	pstd::allocateFixedArena(const size_t size, void* baseAddress) {
	bool precond{ size != 0 };
	if (precond != true) {
		return {};
	}

	pstd::Allocation allocation{ pstd::allocMemory(
		size, pstd::ALLOC_TYPE_COMMIT | pstd::ALLOC_TYPE_RESERVE, baseAddress
	) };

	FixedArena arena{
		.allocation = allocation,
	};
	return arena;
};

void pstd::resetArena(pstd::FixedArena* arena) {
	arena->currentOffset = 0;
	arena->savedOffset = 0;
}

pstd::Allocation<void> pstd::fixedArenaAlloc(
	pstd::FixedArena* arena, const size_t size, const uint32_t alignment
) {
	const size_t initialHeadOffset{ arena->currentOffset };

	bool precond{ (arena->currentOffset + size <= arena->allocation.size) &&
				  (arena->allocation.block != nullptr) && (size != 0) &&
				  (alignment != 0) && (arena != nullptr) &&
				  (arena->currentOffset + size <= arena->allocation.size) };
	ASSERT(precond);

	size_t headAddress{
		((size_t)arena->allocation.block + arena->currentOffset)
	};

	const size_t addressAlignmentDifference{
		(alignment - (headAddress % alignment)) % alignment
	};
	headAddress += addressAlignmentDifference;
	arena->currentOffset += size + addressAlignmentDifference;
	if (arena->currentOffset > arena->allocation.size) {
		arena->currentOffset = initialHeadOffset;
		ASSERT(0);
	}

	pstd::Allocation allocation{ .block = (void*)headAddress, .size = size };
	return allocation;
}

void pstd::destroyFixedArena(pstd::FixedArena* arena) {
	if (arena->allocation.block != nullptr) {
		pstd::freeMemory(arena->allocation.block, pstd::ALLOC_TYPE_RELEASE);
	}
}
