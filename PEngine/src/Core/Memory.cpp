#include "Core/PMemory.h"
#include "Core/PMemory.h"

#include "Core/Memory.h"

#include "Core/PAssert.h"
#include "Core/PAlgorithm.h"
#include "Core/PArena.h"

#include <new>

using namespace pstd;

namespace {
	struct FreelistBlock {
		FreelistBlock* pNext;
		size_t size;
	};

	struct MemoryPool {
		Allocation allocation;
		FreelistBlock* pFirstFreeBlock;
		MemoryPool* pNext;
	};

	MemoryPool* createMemoryPool(size_t size);
	void findSuitableFreeBlock(
		const MemoryPool* pPoolHeader,
		size_t requiredSize,
		FreelistBlock** pOutPrevFreeBlock = nullptr,
		FreelistBlock** pOutFreeBlock = nullptr
	);
}  // namespace

AllocationRegistry pstd::createAllocationRegistry(size_t initialSize) {
	AllocationRegistry registry{ .firstPool = createMemoryPool(initialSize) };
	return registry;
}

Allocation pstd::heapAlloc(
	AllocationRegistry* registry,
	size_t size,
	uint32_t alignment,
	AllocationTypeBits allocType
) {
	ASSERT(registry);
	ASSERT(allocType != ALLOC_INVALID);

	if (!registry->firstPool) {
		registry->firstPool = createMemoryPool(size);
	}

	FreelistBlock* pPrevFreeBlock{};
	FreelistBlock* pSuitableFreeBlock{};

	MemoryPool* pPool{ registry->firstPool };

	findSuitableFreeBlock(pPool, size, &pPrevFreeBlock, &pSuitableFreeBlock);

	MemoryPool* prevPool{};
	while (!pSuitableFreeBlock) {
		prevPool = pPool;
		if (pPool->pNext == nullptr) {
			pPool = createMemoryPool(size);
			prevPool->pNext = pPool;
		} else {
			pPool = pPool->pNext;
		}

		findSuitableFreeBlock(
			pPool, size, &pPrevFreeBlock, &pSuitableFreeBlock
		);
	}
	size_t sizeFree{ pSuitableFreeBlock->size };

	bool isCommitted{};
	if (allocType & ALLOC_COMMITTED) {
		Allocation pageAlloc{
			allocPages(size, ALLOC_COMMITTED, pSuitableFreeBlock)
		};

		ASSERT(pageAlloc.block != nullptr);
		isCommitted = true;
	}

	uint8_t* pUpdatedFreeBlock{};
	size_t newFreeBlockUsableOffset{ size + sizeof(FreelistBlock) };

	if (newFreeBlockUsableOffset < sizeFree) {
		pUpdatedFreeBlock = rcast<uint8_t*>(pSuitableFreeBlock) + size;

		size_t updatedFreeSize{ sizeFree - newFreeBlockUsableOffset };

		allocPages(
			sizeof(FreelistBlock), pstd::ALLOC_COMMITTED, pUpdatedFreeBlock
		);

		new (pUpdatedFreeBlock)
			FreelistBlock{ .pNext = pSuitableFreeBlock->pNext,
						   .size = updatedFreeSize };
	} else {
		pUpdatedFreeBlock = rcast<uint8_t*>(pSuitableFreeBlock->pNext);
	}

	if (pPrevFreeBlock) {
		pPrevFreeBlock->pNext = rcast<FreelistBlock*>(pUpdatedFreeBlock);
	} else {
		pPool->pFirstFreeBlock = rcast<FreelistBlock*>(pUpdatedFreeBlock);
	}

	return Allocation{ .block = rcast<uint8_t*>(pSuitableFreeBlock),
					   .size = size,
					   .ownsMemory = true,
					   .isCommitted = isCommitted };
}

Allocation pstd::heapCommit(void* block, size_t size) {
	Allocation committedAllocation{ allocPages(size, ALLOC_COMMITTED, block) };
	return committedAllocation;
}

void pstd::heapFree(
	AllocationRegistry* registry, const Allocation* allocation
) {}

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

	uint8_t* aEnd{ a.block + a.size };
	uint8_t* bEnd{ b.block + b.size };
	size_t size{ a.size + b.size };
	if (aEnd == b.block) {
		ASSERT(a.block + size == b.block + b.size);
		return Allocation{ .block = a.block,
						   .size = size,
						   .ownsMemory = a.ownsMemory };
	}
	ASSERT(bEnd == a.block);

	ASSERT(a.block + size == a.block + a.size);
	return Allocation{
		.block = b.block,
		.size = size,
		.ownsMemory = b.ownsMemory,
	};
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
		ASSERT(size > 0);

		size += sizeof(MemoryPool) + sizeof(FreelistBlock);

		const Allocation poolAllocation{
			allocPages(size, pstd::ALLOC_RESERVED)
		};

		allocPages(
			sizeof(MemoryPool), pstd::ALLOC_COMMITTED, poolAllocation.block
		);

		MemoryPool* pPoolHeader{ new (poolAllocation.block) MemoryPool{
			.allocation = poolAllocation,
		} };

		uint8_t* pFreelistBlockHeader{ poolAllocation.block +
									   sizeof(MemoryPool) };

		size_t freeSize{ poolAllocation.size - sizeof(MemoryPool) };

		pPoolHeader->pFirstFreeBlock =
			new (pFreelistBlockHeader) FreelistBlock{ .size = freeSize };

		return pPoolHeader;
	}

	void findSuitableFreeBlock(
		const MemoryPool* pPoolHeader,
		size_t requiredSize,
		FreelistBlock** pOutPrevFreeBlock,
		FreelistBlock** pOutFreeBlock
	) {
		FreelistBlock* pPrevFreeBlock{};
		FreelistBlock* pFreeBlock{ pPoolHeader->pFirstFreeBlock };

		while (pFreeBlock) {
			if (pFreeBlock->size >= requiredSize) {
				break;
			}
			pPrevFreeBlock = pFreeBlock;
			pFreeBlock = pFreeBlock->pNext;
		}

		*pOutPrevFreeBlock = pPrevFreeBlock;
		*pOutFreeBlock = pFreeBlock;
	}
}  // namespace
