#pragma once
#include "PTypes.h"

namespace {
	struct MemoryPool;
}

namespace pstd {
	struct Arena;
}

namespace pstd {

	enum AllocationType : uint32_t {
		ALLOC_INVALID = 0b0,
		ALLOC_COMMITTED = 0b1,
		ALLOC_RESERVED = 0b10
	};

	using AllocationTypeBits = uint32_t;

	struct AllocationLimits {
		uint32_t minAllocSize;
		uint32_t pageSize;
	};

	struct AllocationRegistry {
		MemoryPool* firstPool;
	};

	AllocationRegistry
		createAllocationRegistry(size_t initialSize = 1024 * 1024);

	void* heapAlloc(
		AllocationRegistry* state,
		size_t size,
		uint32_t alignment = 16,
		AllocationTypeBits allocType = ALLOC_COMMITTED | ALLOC_RESERVED
	);

	void* heapCommit(void* block, size_t size);

	void heapFree(AllocationRegistry* state, const void* block);

	void memSet(void* dst, int val, size_t size);
	void memZero(void* dst, size_t size);
	void memCpy(void* dst, const void* src, size_t size);
	void memMov(void* dst, const void* src, size_t size);

	AllocationLimits getSystemAllocationLimits();

	size_t roundUpToPageBoundary(size_t size);
	size_t roundDownToPageBoundary(size_t size);

	uint32_t calcAddressAlignmentPadding(
		uintptr_t address, const uint32_t alignment
	);

	template<typename T>
	uint32_t calcAddressAlignmentPadding(uintptr_t address) {
		uint32_t alignment{ alignof(T) };
		return calcAddressAlignmentPadding(address, alignment);
	}

	// template<typename T>
	// size_t getCapacity(const Allocation& allocation) {
	// 	size_t res{ allocation.size / sizeof(T) };
	// 	return res;
	// }

	// void shallowCopy(Allocation* dst, const Allocation& src);
	// void shallowMove(Allocation* dst, const Allocation& src);

	// inline Allocation makeTrunced(Allocation allocation, size_t newSize) {
	// 	ASSERT(newSize <= allocation.size);
	// 	allocation.size = newSize;
	// 	return allocation;
	// }

	// template<typename T>
	// Allocation makeTrunced(const Allocation& allocation, size_t newCount) {
	// 	size_t newSize{ newCount * sizeof(T) };
	// 	return makeTrunced(allocation, newSize);
	// }

	// Allocation makeCoalesced(const Allocation& a, const Allocation& b);
	// bool coalesce(Allocation* a, const Allocation& b);

	// Allocation makeConcatted(
	// 	pstd::Arena* pArena,
	// 	const Allocation& a,
	// 	const Allocation& b,
	// 	uint32_t alignment
	// );

	// template<typename T>
	// Allocation
	// 	makeConcatted(Arena* pArena, const Allocation& a, const Allocation& b) {
	// 	size_t alignment{ alignof(T) };
	// 	return makeConcatted(pArena, a, b, alignment);
	// }

	inline bool isAliasing(
		const uintptr_t aBlock,
		size_t aSize,
		const uintptr_t bBlock,
		size_t bSize
	) {
		ASSERT(aBlock);
		ASSERT(bBlock);

		bool isColliding{ false };
		if (aBlock <= bBlock) {
			isColliding = (aBlock + aSize) > bBlock;
		} else {
			isColliding = (bBlock + bSize) > aBlock;
		}

		return isColliding;
	}

}  // namespace pstd
