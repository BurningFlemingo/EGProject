#include "Core/PMemory.h"
#include "Core/Memory.h"

#include "Core/PTypes.h"
#include "Core/PAssert.h"

#include <Windows.h>

using namespace pstd;

namespace {
	pstd::AllocationLimits g_CachedSysAllocLimits{};
}

size_t pstd::roundUpToPageBoundary(size_t size) {
	AllocationLimits allocLimits{ getSystemAllocationLimits() };

	size_t alignedSize{ ((size + allocLimits.pageSize - 1) /
						 allocLimits.pageSize) *
						allocLimits.pageSize };
	return alignedSize;
}

size_t pstd::roundDownToPageBoundary(size_t size) {
	AllocationLimits allocLimits{ getSystemAllocationLimits() };

	size_t alignedSize{ (size / allocLimits.pageSize) * allocLimits.pageSize };
	return alignedSize;
}

pstd::AllocationLimits pstd::getSystemAllocationLimits() {
	if (g_CachedSysAllocLimits.pageSize != 0) {
		return g_CachedSysAllocLimits;
	}

	SYSTEM_INFO sysInfo{};
	GetSystemInfo(&sysInfo);

	return AllocationLimits{
		.minAllocSize = sysInfo.dwAllocationGranularity,
		.pageSize = sysInfo.dwPageSize,
	};
}

void* pstd::allocPages(
	const size_t size, AllocationTypeBits allocType, void* baseAddress
) {
	ASSERT(allocType != ALLOC_INVALID);

	AllocationLimits allocLimits{ getSystemAllocationLimits() };

	uint32_t win32AllocFlags{};
	uint32_t win32SecurityFlags{ PAGE_NOACCESS };
	bool isCommitted{};

	if (allocType & ALLOC_RESERVED) {
		win32AllocFlags |= MEM_RESERVE;
	}

	if (allocType & ALLOC_COMMITTED) {
		win32AllocFlags |= MEM_COMMIT;
		win32SecurityFlags = PAGE_READWRITE;
		isCommitted = true;
	}

	uint8_t* alignedBaseAddress{
		rcast<uint8_t*>(roundDownToPageBoundary(rcast<size_t>(baseAddress)))
	};

	auto addressPadding{
		ncast<size_t>(ncast<uint8_t*>(baseAddress) - alignedBaseAddress)
	};

	size_t alignedSize{ roundUpToPageBoundary(size + addressPadding) };

	if (alignedSize < allocLimits.minAllocSize &&
		win32AllocFlags & MEM_RESERVE) {
		alignedSize = allocLimits.minAllocSize;
	}

	void* block{ VirtualAlloc(
		alignedBaseAddress, alignedSize, win32AllocFlags, win32SecurityFlags
	) };

	return = block;
}

bool pstd::freePages(
	const Allocation& allocation, AllocationTypeBits allocType
) {
	ASSERT(allocation.ownsMemory);

	uint32_t win32AllocFlags{};
	size_t alignedSize{};
	size_t freeSize{ allocation.size };
	if (allocType == ALLOC_COMMITTED) {
		win32AllocFlags |= MEM_DECOMMIT;
		freeSize = roundUpToPageBoundary(freeSize);
	} else if (allocType == ALLOC_RESERVED) {
		win32AllocFlags |= MEM_RELEASE;
		freeSize = 0;
	}
	if (win32AllocFlags == 0) {
		return 0;
	}

	return VirtualFree(allocation.block, freeSize, win32AllocFlags) != 0;
}
