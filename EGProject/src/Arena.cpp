#include "Arena.h"
#include "PMemory.h"

struct ArenaHeader {
	pstd::Allocation allocation;
};

FixedArena createFixedArena(const size_t size, void* baseAddress) {
	bool precond{ size != 0 };
	if (precond != true) {
		return {};
	}

	size_t allocSize{ sizeof(ArenaHeader) + size };
	pstd::Allocation allocation{ pstd::allocMemory(
		allocSize,
		pstd::ALLOC_TYPE_COMMIT | pstd::ALLOC_TYPE_RESERVE,
		baseAddress
	) };

	auto header{ (ArenaHeader*)allocation.block };

	header->allocation = allocation;

	FixedArena arena{
		.block = header->allocation.block,
		.size = header->allocation.size,
		.headOffset = sizeof(ArenaHeader),
	};
	return arena;
};

void* fixedArenaAlloc(
	FixedArena* arena, const size_t size, const uint32_t alignment
) {
	const size_t initialHeadOffset{ arena->headOffset };

	bool precond{ (arena->headOffset + size <= arena->size) &&
				  (arena->block != nullptr) && (size != 0) &&
				  (alignment != 0) && (arena != nullptr) &&
				  (arena->headOffset + size <= arena->size) };
	if (precond != true) {
		return nullptr;
	}

	size_t headAddress{ ((size_t)arena->block + arena->headOffset) };

	const size_t addressAlignmentDifference{
		(alignment - (headAddress % alignment)) % alignment
	};
	headAddress += addressAlignmentDifference;
	arena->headOffset += size + addressAlignmentDifference;
	if (arena->headOffset > arena->size) {
		arena->headOffset = initialHeadOffset;
		return nullptr;
	}

	return (void*)headAddress;
}

void destroyFixedArena(FixedArena* arena) {
	if (arena->block != nullptr) {
		pstd::freeMemory(arena->block, pstd::ALLOC_TYPE_COMMIT);
	}
}
