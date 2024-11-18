#include "include/PArena.h"
#include "include/PMemory.h"
#include "internal/PMemory.h"
#include "include/PAssert.h"
#include <new>

using namespace pstd;
using namespace pstd::internal;

namespace {
	pstd::Allocation
		bottomAlloc(pstd::Arena* arena, uint32_t size, uint32_t alignment);
	pstd::Allocation
		topAlloc(pstd::Arena* arena, uint32_t size, uint32_t alignment);

	uintptr_t getAlignedBottomOffset(
		uint32_t baseAddress,
		uint32_t bottomOffset,
		uint32_t topOffset,
		uint32_t alignment
	);

	uintptr_t getAlignedTopOffset(
		uint32_t baseAddress,
		uint32_t bottomOffset,
		uint32_t topOffset,
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

	Arena* pTempArena{ pArena };

	if (pState->isFlipped) {
		Allocation allocation{ topAlloc(pTempArena, size, alignment) };
		pArena->topOffset = pTempArena->topOffset;

		pState->scratchOffset = pTempArena->bottomOffset;
		return allocation;
	}

	Allocation allocation{ bottomAlloc(pTempArena, size, alignment) };
	pArena->bottomOffset = pTempArena->bottomOffset;
	pState->scratchOffset = pTempArena->topOffset;
	return allocation;
}

Allocation pstd::scratchAlloc(
	ArenaFrame* arenaFrame, size_t size, uint32_t alignment
) {
	ASSERT(arenaFrame);
	Arena* pArena{ arenaFrame->pArena };
	ArenaFrameState* pState{ &arenaFrame->state };

	Arena* pTempArena{ pArena };
	if (pState->isFlipped) {
		Allocation allocation{ bottomAlloc(pTempArena, size, alignment) };

		pState->scratchOffset = pTempArena->bottomOffset;
		// nothing persists in scratchAlloc

		return allocation;
	}
	Allocation allocation{ topAlloc(pTempArena, size, alignment) };
	pState->scratchOffset = pTempArena->topOffset;
	return allocation;
}

namespace {
	pstd::Allocation bottomAlloc(
		pstd::Arena* arena, const uint32_t size, const uint32_t alignment
	) {
		ASSERT(arena);
		ASSERT(arena->isAllocated);
		ASSERT(arena->allocation.block != nullptr);
		ASSERT(size != 0);
		ASSERT(alignment != 0);

		auto bytesUnaligned{ ncast<uint32_t>(
			(rcast<uintptr_t>(arena->allocation.block) + arena->bottomOffset) %
			alignment
		) };
		uint32_t alignmentPadding{ (alignment - bytesUnaligned) % alignment };

		uint32_t arenaSize{ arena->topOffset - arena->bottomOffset + 1 };
		ASSERT((size + alignmentPadding) <= arenaSize);

		uintptr_t alignedBaseAddress{ rcast<uintptr_t>(arena->allocation.block
									  ) +
									  arena->bottomOffset + alignmentPadding };
		arena->bottomOffset += size + alignmentPadding;

		return Allocation{ .block = rcast<uint8_t*>(alignedBaseAddress),
						   .size = size };
	}

	pstd::Allocation topAlloc(
		pstd::Arena* arena, const uint32_t size, const uint32_t alignment
	) {
		ASSERT(arena);
		ASSERT(arena->isAllocated);
		ASSERT(arena->allocation.block != nullptr);
		ASSERT(size != 0);
		ASSERT(alignment != 0);
		ASSERT(arena->topOffset >= size);
		ASSERT(rcast<uintptr_t>(arena->allocation.block) % alignment == 0);

		auto alignmentPadding{ ncast<uint32_t>(
			rcast<uintptr_t>(
				arena->allocation.block + arena->topOffset - size
			) %
			alignment
		) };
		uint32_t adjustedSize{ size + alignmentPadding };

		ASSERT(arena->topOffset >= arena->bottomOffset + adjustedSize);

		uint8_t* alignedBaseAddress{ arena->allocation.block +
									 arena->topOffset - adjustedSize };
		arena->topOffset -= adjustedSize;

		return Allocation{ .block = alignedBaseAddress, .size = size };
	}

	uintptr_t getAlignedBottomOffset(
		uint32_t baseAddress,
		uint32_t bottomOffset,
		uint32_t topOffset,
		uint32_t alignment
	) {
		uint32_t bytesUnaligned{ (baseAddress + bottomOffset) % alignment };

		uint32_t alignmentPadding{ (alignment - bytesUnaligned) % alignment };

		uintptr_t res{ ((baseAddress + bottomOffset) + alignmentPadding) };
		return res;
	}

	uintptr_t getAlignedTopOffset(
		uint32_t baseAddress,
		uint32_t bottomOffset,
		uint32_t topOffset,
		uint32_t alignment
	) {
		uint32_t alignmentPadding{ (baseAddress + topOffset) % alignment };
		uintptr_t res{ ((baseAddress + topOffset) - alignmentPadding) };
		return res;
	}
}  // namespace
