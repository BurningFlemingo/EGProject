#pragma once
#include "PTypes.h"

namespace pstd {
	struct FixedArena;
}

namespace pstd {

	struct Allocation {
		void* block;
		size_t size;  // always in bytes
		bool ownsMemory;  // if true, memory was allocated from the system
		const bool isStackAllocated;
	};

	struct AllocationLimits {
		uint32_t minAllocSize;
		uint32_t pageSize;
	};

	void memSet(void* dst, int val, size_t size);
	void memZero(void* dst, size_t size);
	void memCpy(void* dst, const void* src, size_t size);
	void memMov(void* dst, const void* src, size_t size);

	AllocationLimits getSystemAllocationLimits();
	size_t alignUpToPageBoundary(size_t size);
	size_t alignDownToPageBoundary(size_t size);

	uint32_t calcAddressAlignmentPadding(
		const void* address, const uint32_t alignment
	);

	template<typename T>
	uint32_t calcAddressAlignmentPadding(const void* address) {
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

	Allocation concat(
		FixedArena* arena,
		const Allocation& a,
		const Allocation& b,
		uint32_t alignment
	);

	template<typename T>
	Allocation
		concat(FixedArena* arena, const Allocation& a, const Allocation& b) {
		return concat(arena, a, b, alignof(T));
	}

}  // namespace pstd
