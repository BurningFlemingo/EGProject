#include "public/PArena.h"
#include "public/PMemory.h"
#include "private/PMemory.h"
#include "public/PAssert.h"
#include <new>

namespace {
	constexpr size_t MIN_ALLOC_SIZE{ 1024 * 1024 *
									 1024 };  // already page aligned

	struct FreelistBlock {
		pstd::Allocation usable;
		FreelistBlock* next;
	};

	struct Freelist {
		pstd::Allocation pool;
		FreelistBlock* firstBlock;
		Freelist* next;
	} g_Freelist;

	Freelist* createFreelist(size_t size);
	void findFreeBlock(
		Freelist* freelist,
		size_t size,
		FreelistBlock** outPrevFreeBlock,
		FreelistBlock** outFreeBlock
	);

}  // namespace

pstd::FixedArena pstd::allocateFixedArena(const size_t size) {
	AllocationLimits allocLimits{ getSystemAllocationLimits() };

	if (!g_Freelist.pool.block) {
		g_Freelist = *createFreelist(size);
	}
	FreelistBlock* prevFreeBlock{};
	FreelistBlock* freeBlock{};

	Freelist* freelist{ &g_Freelist };
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
		return allocateFixedArena(size);  // TODO: make this better
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
		g_Freelist.firstBlock = (FreelistBlock*)newFreeBlockAddr;
	}

	return FixedArena{ .allocation = { .block = freeBlock->usable.block,
									   .size = size },
					   .isAllocated = true };
}
void pstd::freeFixedArena(FixedArena* arena) {
	if (arena->allocation.ownsMemory) {
		pstd::internal::freePages(
			arena->allocation, internal::ALLOC_TYPE_RELEASE
		);
	}
	arena->isAllocated = false;
}

pstd::Allocation pstd::arenaAlloc(
	pstd::FixedArena* arena, const size_t size, const uint32_t alignment
) {
	ASSERT(arena->isAllocated);
	ASSERT(arena != nullptr);
	ASSERT(arena->allocation.block != nullptr);
	ASSERT(size != 0);
	ASSERT(alignment != 0);

	uint32_t alignmentPadding{ pstd::calcAddressAlignmentPadding(
		(void*)((size_t)arena->allocation.block + arena->offset), alignment
	) };

	ASSERT((arena->offset + size + alignmentPadding) <= arena->allocation.size);

	size_t alignedHeadAddress{
		((size_t)arena->allocation.block + arena->offset + alignmentPadding)
	};
	arena->offset += size + alignmentPadding;

	return pstd::Allocation{ .block = (void*)alignedHeadAddress, .size = size };
}

namespace {
	Freelist* createFreelist(size_t size) {
		size_t allocSize{ ((size + MIN_ALLOC_SIZE) / MIN_ALLOC_SIZE) *
						  MIN_ALLOC_SIZE };

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
