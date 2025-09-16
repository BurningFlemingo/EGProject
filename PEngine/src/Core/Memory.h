#pragma once
#include "Core/PMemory.h"
#include "Core/PTypes.h"

namespace pstd {
	struct Allocation {
		uint8_t* block;
		size_t size;  // always in bytes
	};

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
