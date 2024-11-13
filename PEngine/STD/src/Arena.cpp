#include "include/PArena.h"
#include "include/PMemory.h"
#include "internal/PMemory.h"
#include "include/PAssert.h"
#include <new>

using namespace pstd;
using namespace pstd::internal;

namespace {
	pstd::Allocation
		bottomAlloc(pstd::Arena* arena, size_t size, uint32_t alignment);
	pstd::Allocation
		topAlloc(pstd::Arena* arena, size_t size, uint32_t alignment);

	void* getAlignedBottomOffset(
		size_t baseAddress,
		size_t bottomOffset,
		size_t topOffset,
		uint32_t alignment
	);

	void* getAlignedTopOffset(
		size_t baseAddress,
		size_t bottomOffset,
		size_t topOffset,
		uint32_t alignment
	);
}  // namespace

pstd::Arena
	pstd::allocateArena(AllocationRegistry* registry, const size_t size) {
	ASSERT(registry);

	Allocation arenaAllocation{ heapAlloc(registry, size) };

	return Arena{ .allocation = arenaAllocation, .isAllocated = true };
}
void pstd::freeArena(AllocationRegistry* registry, Arena* arena) {
	ASSERT(arena);
	ASSERT(arena->allocation.block);
	if (arena->allocation.ownsMemory) {
		heapFree(registry, &arena->allocation);
	}
	arena->isAllocated = false;
}

Allocation
	pstd::alloc(ArenaFrame* arenaFrame, size_t size, uint32_t alignment) {
	ASSERT(arenaFrame);
	Arena* pArena{ arenaFrame->pArena };
	ArenaFrameState* pState{ &arenaFrame->state };

	Arena tempArena{ pArena };

	if (pState->isFlipped) {
		Allocation allocation{ topAlloc(pArena, size, alignment) };
		pArena->topOffset = tempArena.topOffset;

		pState->scratchOffset = tempArena.bottomOffset;
		return allocation;
	}

	Allocation allocation{ bottomAlloc(pArena, size, alignment) };
	pArena->bottomOffset = tempArena.bottomOffset;
	pState->scratchOffset = tempArena.topOffset;
	return allocation;
}

Allocation pstd::scratchAlloc(
	ArenaFrame* arenaFrame, size_t size, uint32_t alignment
) {
	ASSERT(arenaFrame);
	Arena* pArena{ arenaFrame->pArena };
	ArenaFrameState* pState{ &arenaFrame->state };

	Arena tempArena{ pArena };
	if (pState->isFlipped) {
		Allocation allocation{ bottomAlloc(pArena, size, alignment) };

		pState->scratchOffset = tempArena.bottomOffset;
		// nothing persists in scratchAlloc

		return allocation;
	}
	Allocation allocation{ topAlloc(pArena, size, alignment) };
	pState->scratchOffset = tempArena.topOffset;
	return allocation;
}

void* pstd::getAlignedOffset(const ArenaFrame& frame, uint32_t alignment) {
	size_t baseAddress{ (size_t)frame.pArena->allocation.block };

	if (frame.state.isFlipped) {
		size_t bottomOffset{ frame.state.scratchOffset };
		size_t topOffset{ frame.pArena->topOffset };
		return getAlignedTopOffset(
			baseAddress, bottomOffset, topOffset, alignment
		);
	}
	size_t bottomOffset{ frame.pArena->bottomOffset };
	size_t topOffset{ frame.state.scratchOffset };
	return getAlignedBottomOffset(
		baseAddress, bottomOffset, topOffset, alignment
	);
}

void* pstd::getAlignedScratchOffset(
	const ArenaFrame& frame, uint32_t alignment
) {
	size_t baseAddress{ (size_t)frame.pArena->allocation.block };

	if (frame.state.isFlipped) {
		size_t bottomOffset{ frame.state.scratchOffset };
		size_t topOffset{ frame.pArena->topOffset };
		return getAlignedBottomOffset(
			baseAddress, bottomOffset, topOffset, alignment
		);
	}
	size_t bottomOffset{ frame.pArena->bottomOffset };
	size_t topOffset{ frame.state.scratchOffset };
	return getAlignedTopOffset(baseAddress, bottomOffset, topOffset, alignment);
}

namespace {
	pstd::Allocation bottomAlloc(
		pstd::Arena* arena, const size_t size, const uint32_t alignment
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
		pstd::Arena* arena, const size_t size, const uint32_t alignment
	) {
		ASSERT(arena);
		ASSERT(arena->isAllocated);
		ASSERT(arena->allocation.block != nullptr);
		ASSERT(size != 0);
		ASSERT(alignment != 0);
		ASSERT(arena->topOffset >= size);
		ASSERT((size_t)arena->allocation.block % alignment == 0);

		size_t alignmentPadding{ ((size_t)arena->allocation.block +
								  arena->topOffset - size) %
								 alignment };
		size_t adjustedSize{ size + alignmentPadding };

		ASSERT(arena->topOffset >= arena->bottomOffset + adjustedSize);

		size_t alignedBaseAddress{
			((size_t)arena->allocation.block + arena->topOffset - adjustedSize)
		};
		arena->topOffset -= adjustedSize;

		return Allocation{ .block = (void*)alignedBaseAddress, .size = size };
	}

	void* getAlignedBottomOffset(
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

	void* getAlignedTopOffset(
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
