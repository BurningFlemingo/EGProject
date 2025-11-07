#pragma once
#include "STD/PMemory.h"
#include "STD/PTypes.h"

namespace pstd {
	// returns page aligned block and size, i.e., the block may not
	// equal baseAddress, and allocated size may not equal size
	void* allocPages(
		const size_t size,
		const AllocationTypeBits allocFlags,
		void* baseAddress = nullptr
	);

	// returns true on success and false on failure
	bool freePages(void* block, const AllocationTypeBits allocType);

}  // namespace pstd
