#include "include/PArena.h"
#include "include/PMemory.h"
#include "internal/PMemory.h"
#include "include/PAssert.h"
#include <new>

using namespace pstd;
using namespace pstd::internal;

namespace {
	Allocation bottomAlloc(
		const ArenaFrame& frame,
		const uint32_t size,
		const uint32_t alignment,
		uint32_t* outBottomOffset
	);
	Allocation topAlloc(
		const ArenaFrame& frame,
		const uint32_t size,
		const uint32_t alignment,
		uint32_t* outTopOffset
	);

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

	if (arenaFrame->isFlipped) {
		Allocation allocation{
			topAlloc(*arenaFrame, size, alignment, arenaFrame->pPersistOffset)
		};
		return allocation;
	}

	Allocation allocation{
		bottomAlloc(*arenaFrame, size, alignment, arenaFrame->pPersistOffset)
	};
	return allocation;
}

Allocation pstd::scratchAlloc(
	ArenaFrame* arenaFrame, size_t size, uint32_t alignment
) {
	ASSERT(arenaFrame);
	Arena* pArena{ arenaFrame->pArena };

	if (arenaFrame->isFlipped) {
		Allocation allocation{ bottomAlloc(
			*arenaFrame, size, alignment, &arenaFrame->scratchOffset
		) };
		return allocation;
	}

	Allocation allocation{
		topAlloc(*arenaFrame, size, alignment, &arenaFrame->scratchOffset)
	};
	return allocation;
}

namespace {
	Allocation bottomAlloc(
		const ArenaFrame& frame,
		const uint32_t size,
		const uint32_t alignment,
		uint32_t* outBottomOffset
	) {
		Arena* pArena{ frame.pArena };

		ASSERT(pArena);
		ASSERT(pArena->isAllocated);
		ASSERT(pArena->allocation.block != nullptr);
		ASSERT(frame.pPersistOffset);
		ASSERT(size != 0);
		ASSERT(alignment != 0);

		uint32_t bottomOffset{
			min(frame.scratchOffset, *frame.pPersistOffset)
		};
		uint32_t topOffset{ max(frame.scratchOffset, *frame.pPersistOffset) };

		ASSERT(topOffset >= bottomOffset);

		auto bytesUnaligned{ ncast<uint32_t>(
			(rcast<uintptr_t>(pArena->allocation.block) + bottomOffset) %
			alignment
		) };
		uint32_t alignmentPadding{ (alignment - bytesUnaligned) % alignment };

		uint32_t arenaSize{ topOffset - bottomOffset + 1 };
		ASSERT((size + alignmentPadding) <= arenaSize);

		uintptr_t alignedBaseAddress{ rcast<uintptr_t>(pArena->allocation.block
									  ) +
									  bottomOffset + alignmentPadding };
		bottomOffset += size + alignmentPadding;
		*outBottomOffset = bottomOffset;

		return Allocation{ .block = rcast<uint8_t*>(alignedBaseAddress),
						   .size = size };
	}

	Allocation topAlloc(
		const ArenaFrame& frame,
		const uint32_t size,
		const uint32_t alignment,
		uint32_t* outTopOffset
	) {
		Arena* pArena{ frame.pArena };
		ASSERT(pArena);
		ASSERT(pArena->isAllocated);
		ASSERT(pArena->allocation.block != nullptr);
		ASSERT(frame.pPersistOffset);
		ASSERT(size != 0);
		ASSERT(alignment != 0);
		ASSERT(rcast<uintptr_t>(pArena->allocation.block) % alignment == 0);

		uint32_t bottomOffset{
			min(frame.scratchOffset, *frame.pPersistOffset)
		};
		uint32_t topOffset{ max(frame.scratchOffset, *frame.pPersistOffset) };
		ASSERT(topOffset >= bottomOffset);

		auto alignmentPadding{ ncast<uint32_t>(
			rcast<uintptr_t>(pArena->allocation.block + topOffset - size) %
			alignment
		) };
		uint32_t adjustedSize{ size + alignmentPadding };

		uint32_t arenaSize{ topOffset - bottomOffset + 1 };
		ASSERT(arenaSize >= adjustedSize);

		uint8_t* alignedBaseAddress{ pArena->allocation.block + topOffset -
									 adjustedSize };
		topOffset -= adjustedSize;
		*outTopOffset = topOffset;

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
