#include "include/PArena.h"
#include "include/PMemory.h"
#include "internal/PMemory.h"
#include "include/PAssert.h"
#include <new>

using namespace pstd;
using namespace pstd::internal;

Arena pstd::allocateArena(AllocationRegistry* pAllocRegistry, size_t size) {
	Allocation allocation{ heapAlloc(pAllocRegistry, size) };
	return Arena{ .allocation = allocation };
}

void pstd::freeArena(AllocationRegistry* pAllocRegistry, Arena* pArena) {
	heapFree(pAllocRegistry, &pArena->allocation);
	pArena = {};
}

Allocation pstd::alloc(Arena* pArena, size_t size, uint32_t alignment) {
	ASSERT(pArena);
	ASSERT(pArena->allocation.block != nullptr);
	ASSERT(size != 0);
	ASSERT(alignment != 0);

	uintptr_t baseAddress{ (size_t)pArena->allocation.block };

	uint32_t alignmentPadding{
		(alignment - vcast<uint32_t>((baseAddress + pArena->offset) % alignment)
		) %
		alignment
	};

	uint32_t alignedOffset{ pArena->offset + alignmentPadding };

	ASSERT((size + alignedOffset) <= pArena->allocation.size);

	uintptr_t alignedOffsetAddress{ baseAddress + alignedOffset };
	pArena->offset = alignedOffset + size;

	return Allocation{ .block = rcast<uint8_t*>(alignedOffsetAddress),
					   .size = size };
}
