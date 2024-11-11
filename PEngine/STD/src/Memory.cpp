#include "include/PMemory.h"
#include "include/PMemory.h"

#include "internal/PMemory.h"

#include "include/PAssert.h"
#include "include/PAlgorithm.h"
#include "include/PArena.h"

#include <new>

using namespace pstd;
using namespace pstd::internal;

namespace {
	struct FreelistBlock {
		Allocation usable;
		FreelistBlock* next;
	};

	struct MemoryPool {
		Allocation allocation;
		FreelistBlock* firstFreeBlock;
		MemoryPool* next;
	};

	MemoryPool* createMemoryPool(size_t size);
	void findFreeBlock(
		MemoryPool* pool,
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

	dstBlock += size - 1;
	srcBlock += size - 1;
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

	Allocation allocation{ pstd::alloc(arena, allocSize, alignment) };

	pstd::shallowMove(&allocation, a);

	Allocation headAllocation{ allocation };
	headAllocation.block = (void*)((size_t)headAllocation.block + a.size);
	headAllocation.size -= a.size;

	shallowMove(&headAllocation, b);

	return allocation;
}

AllocationRegistry pstd::createAllocationRegistry(size_t initialSize) {
	AllocationRegistry registry{ .firstPool = createMemoryPool(initialSize) };
	return registry;
}

Allocation
	pstd::internal::heapAlloc(AllocationRegistry* registry, size_t size) {
	ASSERT(registry);

	AllocationLimits allocLimits{ getSystemAllocationLimits() };

	if (!registry->firstPool) {
		registry->firstPool = createMemoryPool(size);
	}
	FreelistBlock* prevFreeBlock{};
	FreelistBlock* freeBlock{};

	MemoryPool* pool{ registry->firstPool };
	findFreeBlock(pool, size, &prevFreeBlock, &freeBlock);

	MemoryPool* prevPool{};
	while (!freeBlock && pool->next) {
		prevPool = pool;
		pool = pool->next;
		findFreeBlock(pool, size, &prevFreeBlock, &freeBlock);
	}
	if (!freeBlock) {
		MemoryPool* newPool{ createMemoryPool(size) };
		if (prevPool) {
			prevPool->next = prevPool;
		} else {
			pool->next = newPool;
		}
		pool = newPool;
		findFreeBlock(pool, size, &prevFreeBlock, &freeBlock);
	}

	void* commitAddr{ (void*)alignDownToPageBoundary((size_t
	)freeBlock->usable.block) };

	size_t commitSize{ alignUpToPageBoundary(
		size + (size_t)freeBlock->usable.block - (size_t)commitAddr
	) };

	size_t commitHeadPtr{ (size_t)commitAddr + commitSize };
	size_t freelistHeadPtr{ (size_t)pool->allocation.block +
							pool->allocation.size };
	if (commitHeadPtr > freelistHeadPtr) {
		pool->next = createMemoryPool(size);
		pool = pool->next;
		return heapAlloc(registry, size);  // TODO: make this better
	}

	allocPages(
		commitSize, ALLOC_TYPE_COMMIT, commitAddr
	);	// TODO: handle tracking, this will overcommit

	void* newFreeBlockAddr{ (void*)((size_t)freeBlock->usable.block + size) };
	new (newFreeBlockAddr)
		FreelistBlock{ .usable = { .block = (void*)((size_t)newFreeBlockAddr +
													sizeof(FreelistBlock)),
								   .size = freeBlock->usable.size - size },
					   .next = freeBlock->next };
	if (prevFreeBlock) {
		prevFreeBlock->next = (FreelistBlock*)newFreeBlockAddr;
	} else {
		registry->firstPool->firstFreeBlock = (FreelistBlock*)newFreeBlockAddr;
	}

	return Allocation{ .block = freeBlock->usable.block,
					   .size = size,
					   .ownsMemory = true };
}

void pstd::internal::heapFree(
	AllocationRegistry* registry, Allocation* allocation
) {}

namespace {
	MemoryPool* createMemoryPool(size_t size) {
		AllocationLimits allocLimits{ pstd::getSystemAllocationLimits() };
		size_t allocSize{ ((size + allocLimits.pageSize) / allocLimits.pageSize
						  ) *
						  allocLimits.pageSize };
		allocSize = max(allocSize, allocLimits.minAllocSize);

		const Allocation poolAllocation{
			allocPages(allocSize, ALLOC_TYPE_RESERVE)
		};
		allocPages(sizeof(MemoryPool), ALLOC_TYPE_COMMIT, poolAllocation.block);

		MemoryPool* pool{ new (poolAllocation.block) MemoryPool{
			.allocation = poolAllocation,
		} };
		size_t freelistHeaderHead{ (size_t)poolAllocation.block +
								   sizeof(MemoryPool) };

		pool->firstFreeBlock =
			new ((void*)freelistHeaderHead) FreelistBlock{ .usable{
				.block = (void*)(freelistHeaderHead + sizeof(FreelistBlock)),
				.size = poolAllocation.size - sizeof(FreelistBlock) -
					sizeof(MemoryPool) } };

		return pool;
	}

	void findFreeBlock(
		MemoryPool* pool,
		size_t size,
		FreelistBlock** outPrevFreeBlock,
		FreelistBlock** outFreeBlock
	) {
		FreelistBlock* prevFreeBlock{ nullptr };
		FreelistBlock* freeBlock{ pool->firstFreeBlock };
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
