#include "Core/PArena.h"
#include "Core/PMemory.h"
#include "Core/Memory.h"
#include "Core/PAssert.h"
#include <new>

using namespace pstd;

// Arena allocation pattern

Arena pstd::allocateArena(AllocationRegistry* pAllocRegistry, size_t size) {
	return Arena{ .block = heapAlloc(pAllocRegistry, size), .size = size };
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

	uint32_t alignmentPadding{
		(alignment - vcast<uint32_t>((baseAddress + pArena->offset) % alignment)
		) %
		alignment
	};

	uint32_t alignedOffset{ pArena->offset + alignmentPadding };

	ASSERT((size + alignedOffset) <= pArena->size);

	uintptr_t alignedOffsetAddress{ baseAddress + alignedOffset };
	pArena->offset = alignedOffset + size;

	return rcast<void*>(alignedOffsetAddress);
}
