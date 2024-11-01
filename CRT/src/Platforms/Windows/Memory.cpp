#include <Windows.h>

#include "public/PMemory.h"
#include "private/PMemory.h"
#include "public/PAssert.h"
#include "public/PAssert.h"

using namespace pstd;

namespace {
	struct MemorySystemState {
		AllocationLimits allocLimits;
	};

	MemorySystemState g_State;

}  // namespace

size_t pstd::alignUpToPageBoundary(size_t size) {
	const MemorySystemState& state{ g_State };

	size_t alignedSize{ ((size + state.allocLimits.pageSize - 1) /
						 state.allocLimits.pageSize) *
						state.allocLimits.pageSize };
	return alignedSize;
}

size_t pstd::alignDownToPageBoundary(size_t size) {
	const MemorySystemState& state{ g_State };

	size_t alignedSize{ (size / state.allocLimits.pageSize) *
						state.allocLimits.pageSize };
	return alignedSize;
}

pstd::Allocation pstd::internal::allocPages(
	const size_t size, AllocationTypeFlagBits allocTypeFlags, void* baseAddress
) {
	ASSERT(~allocTypeFlags & (ALLOC_TYPE_DECOMMIT | ALLOC_TYPE_RELEASE))

	const MemorySystemState& state{ g_State };

	uint32_t win32AllocFlags{};
	uint32_t win32SecurityFlags{ PAGE_NOACCESS };
	if (allocTypeFlags & ALLOC_TYPE_RESERVE) {
		win32AllocFlags |= MEM_RESERVE;
	}
	if (allocTypeFlags & ALLOC_TYPE_COMMIT) {
		win32AllocFlags |= MEM_COMMIT;
		win32SecurityFlags = PAGE_READWRITE;
	}
	if (win32AllocFlags == 0) {
		return {};
	}

	size_t alignedSize{ alignUpToPageBoundary(size) };
	if (alignedSize < state.allocLimits.minAllocSize &&
		win32AllocFlags & MEM_RESERVE) {
		alignedSize = state.allocLimits.minAllocSize;
	}
	void* block{ VirtualAlloc(
		baseAddress, alignedSize, win32AllocFlags, win32SecurityFlags
	) };

	return Allocation{ .block = block,
					   .size = alignedSize,
					   .ownsMemory = true };
	;
}

bool pstd::internal::freePages(
	const Allocation& allocation, AllocationTypeFlagBits allocTypeFlags
) {
	const MemorySystemState& state{ g_State };

	ASSERT(~allocTypeFlags & (ALLOC_TYPE_COMMIT | ALLOC_TYPE_RESERVE))

	ASSERT(((size_t)allocation.block % state.allocLimits.pageSize) == 0);
	ASSERT(allocation.ownsMemory)

	uint32_t win32AllocFlags{};
	size_t alignedSize{};
	size_t freeSize{ allocation.size };
	if (allocTypeFlags & ALLOC_TYPE_DECOMMIT) {
		win32AllocFlags |= MEM_DECOMMIT;
		freeSize = alignUpToPageBoundary(freeSize);
	}
	if (allocTypeFlags & ALLOC_TYPE_RELEASE) {
		win32AllocFlags |= MEM_RELEASE;
		freeSize = 0;
	}
	if (win32AllocFlags == 0) {
		return 0;
	}

	return VirtualFree(allocation.block, freeSize, win32AllocFlags) != 0;
}

pstd::AllocationLimits pstd::getSystemAllocationLimits() {
	SYSTEM_INFO sysInfo{};
	GetSystemInfo(&sysInfo);

	return AllocationLimits{
		.minAllocSize = sysInfo.dwAllocationGranularity,
		.pageSize = sysInfo.dwPageSize,
	};
}

void pstd::internal::initializeMemorySystem() {
	g_State.allocLimits = getSystemAllocationLimits();
}
