#include "public/PMemory.h"
#include "public/PMemory.h"

#include "private/PMemory.h"

#include "PAssert.h"
#include "PAlgorithm.h"
#include "PArena.h"

#include <new>

namespace {
	struct FreelistBlock {
		pstd::Allocation usable;
		FreelistBlock* next;
	};

	struct Freelist {
		pstd::Allocation pool;
		FreelistBlock* firstBlock;
		Freelist* next;
	}* g_Freelist;

	Freelist* createFreelist(size_t size);
	void findFreeBlock(
		Freelist* freelist,
		size_t size,
		FreelistBlock** outPrevFreeBlock,
		FreelistBlock** outFreeBlock
	);
}  // namespace

void pstd::memSet(void* dst, int val, size_t size) {
	memset(dst, val, size);
}
void pstd::memZero(void* dst, size_t size) {
	memset(dst, 0, size);
}
void pstd::memCpy(void* dst, const void* src, size_t size) {
	memcpy(dst, src, size);
}

void pstd::memMov(void* dst, const void* src, size_t size) {
	ASSERT(dst);
	ASSERT(src);

	auto dstBlock{ (char*)dst };
	auto srcBlock = (const char*)src;

	if (dstBlock < srcBlock) {
		while (size) {
			*dstBlock = *srcBlock;
			dstBlock++;
			srcBlock++;
			size--;
		}
		return;
	}

	dstBlock += size;
	srcBlock += size;
	while (size) {
		*dstBlock = *srcBlock;
		dstBlock--;
		srcBlock--;
		size--;
	}
}

uint32_t pstd::calcAddressAlignmentPadding(
	const void* address, const uint32_t alignment
) {
	uint32_t bytesUnaligned{ (uint32_t)((size_t)address % alignment) };
	// the last % alignment is to set the bytes required to align to zero if
	// the address is already aligned
	return (alignment - bytesUnaligned) % alignment;
}

void pstd::shallowCopy(Allocation* dst, const Allocation& src) {
	ASSERT(dst);
	ASSERT(dst->block);
	ASSERT(src.block);

	size_t copySize{ min(src.size, dst->size) };
	memCpy(dst->block, src.block, copySize);
}

void pstd::shallowMove(Allocation* dst, const Allocation& src) {
	ASSERT(dst);
	ASSERT(dst->block);
	ASSERT(src.block);

	size_t moveSize{ min(src.size, dst->size) };
	memMov(dst->block, src.block, moveSize);
}

pstd::Allocation pstd::concat(
	FixedArena* arena,
	const Allocation& a,
	const Allocation& b,
	uint32_t alignment
) {
	ASSERT(arena);
	ASSERT(a.block);
	ASSERT(b.block);
	size_t allocSize{ a.size + b.size };

	ASSERT(allocSize >= a.size);  // overflow check

	Allocation allocation{ pstd::arenaAlloc(arena, allocSize, alignment) };

	pstd::shallowMove(&allocation, a);

	Allocation headAllocation{ allocation };
	headAllocation.block = (void*)((size_t)headAllocation.block + a.size);
	headAllocation.size -= a.size;

	pstd::shallowMove(&headAllocation, b);

	return allocation;
}

pstd::Allocation pstd::internal::heapAlloc(size_t size) {
	AllocationLimits allocLimits{ getSystemAllocationLimits() };

	if (!g_Freelist) {
		g_Freelist = createFreelist(size);
	}
	FreelistBlock* prevFreeBlock{};
	FreelistBlock* freeBlock{};

	Freelist* freelist{ g_Freelist };
	findFreeBlock(freelist, size, &prevFreeBlock, &freeBlock);

	Freelist* prevFreelist{};
	while (!freeBlock && freelist->next) {
		prevFreelist = freelist;
		freelist = freelist->next;
		findFreeBlock(freelist, size, &prevFreeBlock, &freeBlock);
	}
	if (!freeBlock) {
		Freelist* newFreelist{ createFreelist(size) };
		if (prevFreelist) {
			prevFreelist->next = newFreelist;
		} else {
			freelist->next = newFreelist;
		}
		freelist = newFreelist;
		findFreeBlock(freelist, size, &prevFreeBlock, &freeBlock);
	}

	void* commitAddr{ (void*)alignDownToPageBoundary((size_t
	)freeBlock->usable.block) };

	size_t commitSize{ alignUpToPageBoundary(
		size + (size_t)freeBlock->usable.block - (size_t)commitAddr
	) };

	size_t commitHeadPtr{ (size_t)commitAddr + commitSize };
	size_t freelistHeadPtr{ (size_t)freelist->pool.block +
							freelist->pool.size };
	if (commitHeadPtr > freelistHeadPtr) {
		freelist->next = createFreelist(size);
		freelist = freelist->next;
		return heapAlloc(size);	 // TODO: make this better
	}

	pstd::internal::allocPages(
		commitSize, internal::ALLOC_TYPE_COMMIT, commitAddr
	);	// TODO: handle tracking, this will overcommit
		// BUG: this can commit outside of reserved range

	void* newFreeBlockAddr{ (void*)((size_t)freeBlock->usable.block + size) };
	new (newFreeBlockAddr)
		FreelistBlock{ .usable = { .block = (void*)((size_t)newFreeBlockAddr +
													sizeof(FreelistBlock)),
								   .size = freeBlock->usable.size - size },
					   .next = freeBlock->next };
	if (prevFreeBlock) {
		prevFreeBlock->next = (FreelistBlock*)newFreeBlockAddr;
	} else {
		g_Freelist->firstBlock = (FreelistBlock*)newFreeBlockAddr;
	}

	return Allocation{ .block = freeBlock->usable.block,
					   .size = size,
					   .ownsMemory = true };
}

void pstd::internal::heapFree(Allocation* allocation) {}

namespace {
	Freelist* createFreelist(size_t size) {
		pstd::AllocationLimits allocLimits{ pstd::getSystemAllocationLimits() };
		size_t allocSize{ ((size + allocLimits.pageSize) / allocLimits.pageSize
						  ) *
						  allocLimits.pageSize };
		allocSize = max(allocSize, allocLimits.minAllocSize);

		const pstd::Allocation poolAllocation{ pstd::internal::allocPages(
			allocSize, pstd::internal::ALLOC_TYPE_RESERVE
		) };
		pstd::internal::allocPages(
			sizeof(Freelist),
			pstd::internal::ALLOC_TYPE_COMMIT,
			poolAllocation.block
		);

		Freelist* freelist{ new (poolAllocation.block) Freelist{
			.pool = poolAllocation,
		} };
		size_t freelistHeaderHead{ (size_t)poolAllocation.block +
								   sizeof(Freelist) };

		freelist->firstBlock =
			new ((void*)freelistHeaderHead) FreelistBlock{ .usable{
				.block = (void*)(freelistHeaderHead + sizeof(FreelistBlock)),
				.size = poolAllocation.size - sizeof(FreelistBlock) -
					sizeof(Freelist) } };

		return freelist;
	}

	void findFreeBlock(
		Freelist* freelist,
		size_t size,
		FreelistBlock** outPrevFreeBlock,
		FreelistBlock** outFreeBlock
	) {
		FreelistBlock* prevFreeBlock{ nullptr };
		FreelistBlock* freeBlock{ freelist->firstBlock };
		while (freeBlock) {
			if (freeBlock->usable.size >= size) {
				break;
			}
			prevFreeBlock = freeBlock;
			freeBlock = freeBlock->next;
		}

		*outPrevFreeBlock = prevFreeBlock;
		*outFreeBlock = freeBlock;
	}
}  // namespace
