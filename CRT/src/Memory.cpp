#include "private/PMemory.h"
#include "public/PMemory.h"
#include "public/PMemory.h"
#include "public/PAssert.h"

pstd::Allocation pstd::heapAlloc(const size_t size, void* baseAddress) {
	pstd::Allocation allocation{ pstd::allocPages(
		size, ALLOC_TYPE_COMMIT | ALLOC_TYPE_RESERVE, baseAddress
	) };
	return allocation;
}

void pstd::heapFree(pstd::Allocation* allocation) {
	ASSERT(allocation);

	pstd::freePages(*allocation, ALLOC_TYPE_RELEASE);
	allocation->block = nullptr;
}
