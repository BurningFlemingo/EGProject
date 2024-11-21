#pragma once
#include "PTypes.h"

namespace {
	struct MemoryPool;
}

namespace pstd {
	struct ArenaFrame;
}

namespace pstd {

	struct Allocation {
		uint8_t* block;
		size_t size;  // always in bytes
		bool ownsMemory;  // if true, memory was allocated from the system
		bool isStackAllocated;
	};

	struct AllocationLimits {
		uint32_t minAllocSize;
		uint32_t pageSize;
	};

	struct AllocationRegistry {
		MemoryPool* firstPool;
	};

	AllocationRegistry
		createAllocationRegistry(size_t initialSize = 1024 * 1024);

	void memSet(void* dst, int val, size_t size);
	void memZero(void* dst, size_t size);
	void memCpy(void* dst, const void* src, size_t size);
	void memMov(void* dst, const void* src, size_t size);

	AllocationLimits getSystemAllocationLimits();

	size_t alignUpToPageBoundary(size_t size);
	size_t alignDownToPageBoundary(size_t size);

	uint32_t calcAddressAlignmentPadding(
		uintptr_t address, const uint32_t alignment
	);

	template<typename T>
	uint32_t calcAddressAlignmentPadding(uintptr_t address) {
		uint32_t alignment{ alignof(T) };
		return calcAddressAlignmentPadding(address, alignment);
	}

	template<typename T>
	size_t getCapacity(const Allocation& allocation) {
		size_t res{ allocation.size / sizeof(T) };
		return res;
	}

	void shallowCopy(Allocation* dst, const Allocation& src);
	void shallowMove(Allocation* dst, const Allocation& src);

	inline Allocation makeTrunced(Allocation allocation, size_t newSize) {
		ASSERT(newSize <= allocation.size);
		allocation.size = newSize;
		return allocation;
	}

	template<typename T>
	Allocation makeTrunced(const Allocation& allocation, size_t newCount) {
		size_t newSize{ newCount * sizeof(T) };
		return makeTrunced(allocation, newSize);
	}

	Allocation coalesce(const Allocation& a, const Allocation& b);

}  // namespace pstd
