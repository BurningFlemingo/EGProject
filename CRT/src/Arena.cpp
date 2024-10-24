#include "public/PArena.h"
#include "public/PMemory.h"
#include "public/PAssert.h"

namespace {
	uint32_t calcAddressAlignmentPadding(
		const void* address, const uint32_t alignment
	) {
		uint32_t bytesUnaligned{ (uint32_t)((size_t)address % alignment) };
		// the last % alignment is to set the bytes required to align to zero if
		// the address is already aligned
		uint32_t bytesRequiredToAlign{ (alignment - bytesUnaligned) %
									   alignment };
		return bytesRequiredToAlign;
	}

}  // namespace

pstd::Allocation pstd::bufferAlloc(
	pstd::FixedArena* arena, const size_t size, const uint32_t alignment
) {
	ASSERT(arena != nullptr);
	ASSERT(arena->allocation.block != nullptr);
	ASSERT(size != 0);
	ASSERT(alignment != 0);

	uint32_t alignmentPadding{
		calcAddressAlignmentPadding(arena->allocation.block, alignment)
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
