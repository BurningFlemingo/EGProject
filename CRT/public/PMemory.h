#pragma once
#include "PTypes.h"

namespace pstd {
	enum AllocationType : uint32_t {
		ALLOC_TYPE_COMMIT = 1,
		ALLOC_TYPE_RESERVE = 2,
		ALLOC_TYPE_DECOMMIT = 4,
		ALLOC_TYPE_RELEASE = 8,
	};
	using AllocationTypeFlagBits = uint32_t;

	struct Allocation {
		void* block;
		size_t size;  // always in bytes
	};

	struct AllocationLimits {
		uint32_t minAllocSize;
		uint32_t pageSize;
	};

	Allocation heapAlloc(const size_t size, void* baseAddress = nullptr);

	template<typename T>
	Allocation heapAlloc(const size_t count, void* baseAddress = nullptr) {
		const size_t bytesToAllocate{ count * sizeof(T) };
		Allocation allocation{ heapAlloc(bytesToAllocate, baseAddress) };
		return allocation;
	}
	void heapFree(Allocation* allocation);

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
