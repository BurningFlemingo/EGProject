#pragma once
#include <stdint.h>

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

	Allocation allocMemory(
		const size_t size,
		const AllocationTypeFlagBits allocTypeFlags,
		void* baseAddress = nullptr
	);

	// returns true on success and false on failure
	bool freeMemory(
		void* baseAddress,
		const AllocationTypeFlagBits allocTypeFlags,
		const size_t size = 0
	);

	void setMemory(void* dst, int val, size_t size);
	void zeroMemory(void* dst, size_t size);
	void cpyMemory(void* dst, const void* src, size_t size);

	AllocationLimits getSystemAllocationLimits();
	size_t alignToPageBoundary(size_t size);

}  // namespace pstd
