#pragma once
#include "Core/PMemory.h"
#include "Core/PTypes.h"

namespace pstd {
	// returns page aligned block and size, i.e., allocation.block may not
	// equal baseAddress, and allocation.size may not equal size
	Allocation allocPages(
		const size_t size,
		const AllocationTypeBits allocFlags,
		void* baseAddress = nullptr
	);

	// returns true on success and false on failure
	bool freePages(
		const Allocation& allocation, const AllocationTypeBits allocType
	);

}  // namespace pstd
