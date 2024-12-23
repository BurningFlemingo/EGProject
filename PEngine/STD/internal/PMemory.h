#pragma once
#include "STD/include/PMemory.h"
#include "STD/include/PTypes.h"

namespace pstd {
	namespace internal {
		enum AllocationType : uint32_t {
			ALLOC_TYPE_COMMIT = 1,
			ALLOC_TYPE_RESERVE = 2,
			ALLOC_TYPE_DECOMMIT = 4,
			ALLOC_TYPE_RELEASE = 8,
		};
		using AllocationTypeFlagBits = uint32_t;

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

		Allocation heapAlloc(AllocationRegistry* state, size_t size);
		void heapFree(AllocationRegistry* state, const Allocation* allocation);
	}  // namespace internal
}  // namespace pstd
