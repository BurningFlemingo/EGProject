#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "STD/Memory.h"
#include "STD/PAssert.h"
#include <new>

using namespace pstd;

// Arena allocation pattern

Arena pstd::allocateArena(AllocationRegistry* pAllocRegistry, size_t size) {
	return Arena{ .block = heapAlloc(pAllocRegistry, size), .size = size };
}

Arena pstd::createArena(Arena* pArena, size_t size) {
	return Arena{ .block = alloc(pArena, size, 8), .size = size };
}

void pstd::freeArena(AllocationRegistry* pAllocRegistry, Arena* pArena) {
	heapFree(pAllocRegistry, &pArena->block);
	pArena = {};
}

void* pstd::alloc(Arena* pArena, size_t size, uint32_t alignment) {
	ASSERT(pArena);
	ASSERT(pArena->block != nullptr);
	ASSERT(size != 0);
	ASSERT(alignment != 0);

	auto baseAddress{ rcast<uintptr_t>(pArena->block) };

	size_t alignmentPadding{
		(alignment - vcast<uint32_t>((baseAddress + pArena->offset) % alignment)
		) %
		alignment
	};

	size_t alignedOffset{ pArena->offset + alignmentPadding };

	ASSERT((size + alignedOffset) <= pArena->size);

	void* alignedOffsetAddress{ rcast<void*>(baseAddress + alignedOffset) };

	pArena->offset = alignedOffset + size;
	return alignedOffsetAddress;
}
