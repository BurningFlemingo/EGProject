#pragma once
#include "PTypes.h"

namespace pstd {

	struct Allocation {
		void* block;
		size_t size;  // always in bytes
		bool ownsMemory;  // if true, memory was allocated from the system
	};

	struct AllocationLimits {
		uint32_t minAllocSize;
		uint32_t pageSize;
	};

	void memSet(void* dst, int val, size_t size);
	void memZero(void* dst, size_t size);
	void memCpy(void* dst, const void* src, size_t size);

	AllocationLimits getSystemAllocationLimits();
	size_t alignToPageBoundary(size_t size);

	uint32_t calcAddressAlignmentPadding(
		const void* address, const uint32_t alignment
	);

	template<typename T>
	uint32_t calcAddressAlignmentPadding(const void* address) {
		uint32_t alignment{ alignof(T) };
		uint32_t res{ calcAddressAlignmentPadding(address, alignment) };
		return res;
	}

}  // namespace pstd
