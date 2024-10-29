#pragma once
#include "public/PMemory.h"

namespace pstd {
	namespace internal {
		enum AllocationType : uint32_t {
			ALLOC_TYPE_COMMIT = 1,
			ALLOC_TYPE_RESERVE = 2,
			ALLOC_TYPE_DECOMMIT = 4,
			ALLOC_TYPE_RELEASE = 8,
		};
		using AllocationTypeFlagBits = uint32_t;

		void initializeMemorySystem();

		pstd::Allocation allocPages(
			const size_t size,
			const AllocationTypeFlagBits allocTypeFlags,
			void* baseAddress = nullptr
		);

		// returns true on success and false on failure
		bool freePages(
			const pstd::Allocation& allocation,
			const AllocationTypeFlagBits allocTypeFlags
		);

	}  // namespace internal
}  // namespace pstd
