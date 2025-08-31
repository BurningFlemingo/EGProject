#pragma once
#include "STD/include/PMemory.h"
#include "STD/include/PTypes.h"

namespace pstd {
	namespace internal {

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

	}  // namespace internal
}  // namespace pstd
