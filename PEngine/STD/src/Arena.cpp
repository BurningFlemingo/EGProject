#include "include/PArena.h"
#include "include/PMemory.h"
#include "internal/PMemory.h"
#include "include/PAssert.h"
#include <new>

using namespace pstd;
using namespace pstd::internal;

namespace {
	pstd::Allocation
		bottomAlloc(pstd::FixedArena* arena, size_t size, uint32_t alignment);
	pstd::Allocation
		topAlloc(pstd::FixedArena* arena, size_t size, uint32_t alignment);

	void* getNextForwardAllocAddress(
		size_t baseAddress,
		size_t bottomOffset,
		size_t topOffset,
		uint32_t alignment
	);

	void* getNextBackwardAllocAddress(
		size_t baseAddress,
		size_t bottomOffset,
		size_t topOffset,
		uint32_t alignment
	);
}  // namespace

pstd::FixedArena
	pstd::allocateFixedArena(AllocationRegistry* registry, const size_t size) {
	ASSERT(registry);

	Allocation arenaAllocation{ heapAlloc(registry, size) };

	return FixedArena{ .allocation = arenaAllocation, .isAllocated = true };
}
void pstd::freeFixedArena(AllocationRegistry* registry, FixedArena* arena) {
	ASSERT(arena);
	ASSERT(arena->allocation.block);
	if (arena->allocation.ownsMemory) {
		heapFree(registry, &arena->allocation);
	}
	arena->isAllocated = false;
}

Allocation
	pstd::alloc(FixedArenaFrame* arenaFrame, size_t size, uint32_t alignment) {
	ASSERT(arenaFrame);
	FixedArena* pArena{ arenaFrame->pArena };
	StackFrame* pFrame{ &arenaFrame->frame };
	// saved to preserve original arenas returns
	FixedArena tempArena{ pArena };
	// when flipped, the stack begins at the bottom and the returns go at
	// the top
	if (pFrame->isFlipped) {
		Allocation allocation{ topAlloc(pArena, size, alignment) };
		// we dont want "frame"'s stack to persist after frame gets
		// popped, so we save only to the frame
		pFrame->stackOffset = tempArena.bottomOffset;

		// we want to have the returns persist after "frame" gets
		// popped
		pArena->topOffset =
			tempArena.topOffset;  // saving the return region offset
								  // thru the arena's pointer which
								  // lives longer than "frame"

		return allocation;
	}
	// normally, the stack begins at the top and the returns go at
	// the bottom
	Allocation allocation{ bottomAlloc(pArena, size, alignment) };
	pFrame->stackOffset = tempArena.topOffset;
	pArena->bottomOffset = tempArena.bottomOffset;
	return allocation;
}

Allocation pstd::scratchAlloc(
	FixedArenaFrame* arenaFrame, size_t size, uint32_t alignment
) {
	ASSERT(arenaFrame);
	FixedArena* pArena{ arenaFrame->pArena };
	StackFrame* pFrame{ &arenaFrame->frame };

	FixedArena tempArena{ pArena };
	if (pFrame->isFlipped) {
		Allocation allocation{ bottomAlloc(pArena, size, alignment) };
		pFrame->stackOffset = tempArena.bottomOffset;

		pArena->topOffset = tempArena.topOffset;

		return allocation;
	}
	Allocation allocation{ topAlloc(pArena, size, alignment) };
	pFrame->stackOffset = tempArena.topOffset;
	pArena->bottomOffset = tempArena.bottomOffset;
	return allocation;
}

void* pstd::getNextAllocAddress(
	const FixedArenaFrame& arenaFrame, uint32_t alignment
) {
	const auto& [arena, frame] = arenaFrame;

	size_t baseAddress{ (size_t)arena->allocation.block };

	if (frame.isFlipped) {
		size_t bottomOffset{ frame.stackOffset };
		size_t topOffset{ arena->topOffset };
		return getNextBackwardAllocAddress(
			baseAddress, bottomOffset, topOffset, alignment
		);
	}
	size_t bottomOffset{ arena->bottomOffset };
	size_t topOffset{ frame.stackOffset };
	return getNextForwardAllocAddress(
		baseAddress, bottomOffset, topOffset, alignment
	);
}

void* pstd::getNextScratchAllocAddress(
	const FixedArenaFrame& arenaFrame, uint32_t alignment
) {
	const auto& [arena, frame] = arenaFrame;

	size_t baseAddress{ (size_t)arena->allocation.block };

	if (frame.isFlipped) {
		size_t bottomOffset{ frame.stackOffset };
		size_t topOffset{ arena->topOffset };
		return getNextForwardAllocAddress(
			baseAddress, bottomOffset, topOffset, alignment
		);
	}
	size_t bottomOffset{ arena->bottomOffset };
	size_t topOffset{ frame.stackOffset };
	return getNextBackwardAllocAddress(
		baseAddress, bottomOffset, topOffset, alignment
	);
}

namespace {
	pstd::Allocation bottomAlloc(
		pstd::FixedArena* arena, const size_t size, const uint32_t alignment
	) {
		ASSERT(arena);
		ASSERT(arena->isAllocated);
		ASSERT(arena->allocation.block != nullptr);
		ASSERT(size != 0);
		ASSERT(alignment != 0);

		size_t bytesUnaligned{
			((size_t)arena->allocation.block + arena->bottomOffset) % alignment
		};
		size_t alignmentPadding{ (alignment - bytesUnaligned) % alignment };

		ASSERT(
			(arena->bottomOffset + size + alignmentPadding) <= arena->topOffset
		);

		size_t alignedBaseAddress{
			((size_t)arena->allocation.block + arena->bottomOffset +
			 alignmentPadding)
		};
		arena->bottomOffset += size + alignmentPadding;

		return Allocation{ .block = (void*)alignedBaseAddress, .size = size };
	}

	pstd::Allocation topAlloc(
		pstd::FixedArena* arena, const size_t size, const uint32_t alignment
	) {
		ASSERT(arena);
		ASSERT(arena->isAllocated);
		ASSERT(arena->allocation.block != nullptr);
		ASSERT(size != 0);
		ASSERT(alignment != 0);

		size_t alignmentPadding{
			((size_t)arena->allocation.block + arena->topOffset) % alignment
		};
		size_t adjustedSize{ size + alignmentPadding };

		ASSERT(arena->topOffset >= arena->bottomOffset + adjustedSize);

		size_t alignedBaseAddress{
			((size_t)arena->allocation.block + arena->topOffset - adjustedSize)
		};
		arena->topOffset -= adjustedSize;

		return Allocation{ .block = (void*)alignedBaseAddress, .size = size };
	}

	void* getNextForwardAllocAddress(
		size_t baseAddress,
		size_t bottomOffset,
		size_t topOffset,
		uint32_t alignment
	) {
		size_t bytesUnaligned{ (baseAddress + bottomOffset) % alignment };

		size_t alignmentPadding{ (alignment - bytesUnaligned) % alignment };

		void* res{ (void*)((baseAddress + bottomOffset) + alignmentPadding) };
		return res;
	}

	void* getNextBackwardAllocAddress(
		size_t baseAddress,
		size_t bottomOffset,
		size_t topOffset,
		uint32_t alignment
	) {
		size_t alignmentPadding{ (baseAddress + topOffset) % alignment };
		void* res{ (void*)((baseAddress + topOffset) - alignmentPadding) };
		return res;
	}
}  // namespace
