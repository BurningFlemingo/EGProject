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
		const MemoryPool* pool,
		size_t requiredSize,
		FreelistBlock** outPrevFreeBlock,
		FreelistBlock** outFreeBlock
	);
}  // namespace

AllocationRegistry pstd::createAllocationRegistry(size_t initialSize) {
	AllocationRegistry registry{ .firstPool = createMemoryPool(initialSize) };
	return registry;
}

Allocation pstd::heapAlloc(AllocationRegistry* registry, size_t size) {
	ASSERT(registry);
	size = alignUpToPageBoundary(size + sizeof(FreelistBlock));

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
			prevPool->next = newPool;
		} else {
			pool->next = newPool;
		}
		pool = newPool;
		findFreeBlock(pool, size, &prevFreeBlock, &freeBlock);
	}

	auto* commitAddr{ rcast<uint8_t*>(
		alignDownToPageBoundary(rcast<size_t>(freeBlock->usable.block))
	) };

	size_t commitSize{
		alignUpToPageBoundary(size + freeBlock->usable.block - commitAddr)
	};

	uint8_t* commitHeadPtr{ commitAddr + commitSize };
	uint8_t* freeBlockHeadPtr{ freeBlock->usable.block +
							   freeBlock->usable.size };
	ASSERT(commitHeadPtr <= freeBlockHeadPtr);

	allocPages(
		commitSize, ALLOC_TYPE_COMMIT, commitAddr
	);	// TODO: handle tracking, this will overcommit

	uint8_t* newFreeBlockAddr{ freeBlock->usable.block + size };
	uint8_t* poolHeadAddr{ pool->allocation.block + pool->allocation.size };
	if ((newFreeBlockAddr + sizeof(FreelistBlock) + 1) < poolHeadAddr) {
		new (newFreeBlockAddr) FreelistBlock{
			.usable = { .block = newFreeBlockAddr + sizeof(FreelistBlock),
						.size = freeBlock->usable.size - size -
							sizeof(FreelistBlock) },
			.next = freeBlock->next
		};
	}

	if (prevFreeBlock) {
		prevFreeBlock->next = (FreelistBlock*)newFreeBlockAddr;
	} else {
		pool->firstFreeBlock = (FreelistBlock*)newFreeBlockAddr;
	}

	return Allocation{ .block = freeBlock->usable.block,
					   .size = size,
					   .ownsMemory = true };
}

void pstd::heapFree(
	AllocationRegistry* registry, const Allocation* allocation
) {
	// TODO: implement
}

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
	uintptr_t address, const uint32_t alignment
) {
	auto bytesUnaligned{ ncast<uint32_t>(address % alignment) };
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

bool pstd::coalesce(Allocation* a, const Allocation& b) {
	ASSERT(a);
	*a = makeCoalesced(*a, b);
	if (a->block) {
		return false;
	}
	return true;
}

Allocation pstd::makeCoalesced(const Allocation& a, const Allocation& b) {
	ASSERT(~(a.ownsMemory ^ b.ownsMemory));
	ASSERT(~(a.isStackAllocated ^ b.isStackAllocated));

	uint8_t* aEnd{ a.block + a.size };
	uint8_t* bEnd{ b.block + b.size };
	size_t size{ a.size + b.size };
	if (aEnd == b.block) {
		ASSERT(a.block + size == b.block + b.size);
		return Allocation{ .block = a.block,
						   .size = size,
						   .ownsMemory = a.ownsMemory,
						   .isStackAllocated = a.isStackAllocated };
	}
	ASSERT(bEnd == a.block);

	ASSERT(a.block + size == a.block + a.size);
	return Allocation{ .block = b.block,
					   .size = size,
					   .ownsMemory = b.ownsMemory,
					   .isStackAllocated = b.isStackAllocated };
}

pstd::Allocation pstd::makeConcatted(
	Arena* pArena, const Allocation& a, const Allocation& b, uint32_t alignment
) {
	ASSERT(a.block);
	ASSERT(b.block);
	size_t allocSize{ a.size + b.size };

	ASSERT(allocSize >= a.size);  // overflow check

	Allocation allocation{ pstd::alloc(pArena, allocSize, alignment) };

	pstd::shallowMove(&allocation, a);

	Allocation headAllocation{ allocation };
	headAllocation.block = headAllocation.block + a.size;
	headAllocation.size -= a.size;

	shallowMove(&headAllocation, b);

	return allocation;
}

namespace {
	MemoryPool* createMemoryPool(size_t size) {
		AllocationLimits allocLimits{ pstd::getSystemAllocationLimits() };
		size += sizeof(FreelistBlock);

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
		uint8_t* freelistHeaderHead{ poolAllocation.block +
									 sizeof(MemoryPool) };

		pool->firstFreeBlock = new (freelistHeaderHead) FreelistBlock{ .usable{
			.block = freelistHeaderHead + sizeof(FreelistBlock),
			.size = poolAllocation.size - sizeof(FreelistBlock) -
				sizeof(MemoryPool) } };

		return pool;
	}

	void findFreeBlock(
		const MemoryPool* pool,
		size_t requiredSize,
		FreelistBlock** outPrevFreeBlock,
		FreelistBlock** outFreeBlock
	) {
		FreelistBlock* prevFreeBlock{ nullptr };
		FreelistBlock* freeBlock{ pool->firstFreeBlock };
		while (freeBlock) {
			if (freeBlock->usable.size >= requiredSize) {
				break;
			}
			prevFreeBlock = freeBlock;
			freeBlock = freeBlock->next;
		}

		*outPrevFreeBlock = prevFreeBlock;
		*outFreeBlock = freeBlock;
	}
}  // namespace
