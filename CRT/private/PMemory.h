#pragma once
#include "public/PMemory.h"

namespace _pstd {
	void initializeMemorySystem();

	pstd::Allocation allocPages(
		const size_t size,
		const pstd::AllocationTypeFlagBits allocTypeFlags,
		void* baseAddress = nullptr
	);

	// returns true on success and false on failure
	bool freePages(
		const pstd::Allocation& allocation,
		const pstd::AllocationTypeFlagBits allocTypeFlags
	);

	pstd::Allocation heapAlloc(const size_t size, void* baseAddress = nullptr);

	template<typename T>
	pstd::Allocation
		heapAlloc(const size_t count, void* baseAddress = nullptr) {
		const size_t bytesToAllocate{ count * sizeof(T) };
		pstd::Allocation allocation{ heapAlloc(bytesToAllocate, baseAddress) };
		return allocation;
	}

	void heapFree(pstd::Allocation* allocation);
}  // namespace _pstd
