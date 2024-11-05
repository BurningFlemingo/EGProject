#pragma once
#include "../public/PMemory.h"
#include "../public/PTypes.h"
#include "PCRTAPI.h"

namespace pcrt {
	enum AllocationType : uint32_t {
		ALLOC_TYPE_COMMIT = 1,
		ALLOC_TYPE_RESERVE = 2,
		ALLOC_TYPE_DECOMMIT = 4,
		ALLOC_TYPE_RELEASE = 8,
	};
	using AllocationTypeFlagBits = uint32_t;

	PCRT_API void initializeMemorySystem();

	PCRT_API pstd::Allocation allocPages(
		const size_t size,
		const AllocationTypeFlagBits allocTypeFlags,
		void* baseAddress = nullptr
	);

	// returns true on success and false on failure
	PCRT_API bool freePages(
		const pstd::Allocation& allocation,
		const AllocationTypeFlagBits allocTypeFlags
	);

	PCRT_API pstd::Allocation heapAlloc(size_t size);
	PCRT_API void heapFree(pstd::Allocation* allocation);

}  // namespace pcrt
