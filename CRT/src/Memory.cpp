#include "private/PMemory.h"
#include "public/PMemory.h"
#include "public/PMemory.h"
#include "public/PAssert.h"

pstd::Allocation
	pstd::internal::heapAlloc(const size_t size, void* baseAddress) {
	pstd::Allocation allocation{
		allocPages(size, ALLOC_TYPE_COMMIT | ALLOC_TYPE_RESERVE, baseAddress)
	};
	return allocation;
}

void pstd::internal::heapFree(pstd::Allocation* allocation) {
	ASSERT(allocation);

	freePages(*allocation, ALLOC_TYPE_RELEASE);
	allocation->block = nullptr;
}

uint32_t pstd::calcAddressAlignmentPadding(
	const void* address, const uint32_t alignment
) {
	uint32_t bytesUnaligned{ (uint32_t)((size_t)address % alignment) };
	// the last % alignment is to set the bytes required to align to zero if
	// the address is already aligned
	uint32_t bytesRequiredToAlign{ (alignment - bytesUnaligned) % alignment };
	return bytesRequiredToAlign;
}
