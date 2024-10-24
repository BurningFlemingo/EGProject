#pragma once
#include "public/PMemory.h"

namespace pstd {
	void initializeMemorySystem();
	void cleanupMemorySystem();

	Allocation allocPages(
		const size_t size,
		const AllocationTypeFlagBits allocTypeFlags,
		void* baseAddress = nullptr
	);

	// returns true on success and false on failure
	bool freePages(
		const Allocation& allocation,
		const AllocationTypeFlagBits allocTypeFlags
	);
}  // namespace pstd
