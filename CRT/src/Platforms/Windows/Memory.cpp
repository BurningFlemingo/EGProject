#include "PMemory.h"
#include "private/PMemory.h"
#include <Windows.h>
#include "PAssert.h"
#include "PAssert.h"

using namespace pstd;

namespace {
	struct MemorySystemState {
		AllocationLimits allocLimits;
	};

	MemorySystemState g_State;

}  // namespace

void pstd::cleanupMemorySystem() {}

size_t pstd::alignToPageBoundary(size_t size) {
	const MemorySystemState& state{ g_State };

	size_t alignedSize{ ((size + state.allocLimits.pageSize - 1) /
						 state.allocLimits.pageSize) *
						state.allocLimits.pageSize };
	return alignedSize;
}

pstd::Allocation<void> pstd::allocMemory(
	const size_t size, AllocationTypeFlagBits allocTypeFlags, void* baseAddress
) {
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

	size_t alignedSize{ alignToPageBoundary(size) };
	if (alignedSize < state.allocLimits.minAllocSize &&
		win32AllocFlags & MEM_RESERVE) {
		alignedSize = state.allocLimits.minAllocSize;
	}
	void* block{ VirtualAlloc(
		baseAddress, alignedSize, win32AllocFlags, win32SecurityFlags
	) };
	Allocation allocation{ .block = block, .size = alignedSize };

	return allocation;
}

bool pstd::freeMemory(
	void* block, AllocationTypeFlagBits allocTypeFlags, size_t size
) {
	const MemorySystemState& state{ g_State };

	ASSERT(~allocTypeFlags & (ALLOC_TYPE_COMMIT | ALLOC_TYPE_RESERVE))

	ASSERT(((size_t)block % state.allocLimits.pageSize) == 0);

	uint32_t win32AllocFlags{};
	size_t alignedSize{};
	if (allocTypeFlags & ALLOC_TYPE_DECOMMIT) {
		win32AllocFlags |= MEM_DECOMMIT;
		size = alignToPageBoundary(size);
	}
	if (allocTypeFlags & ALLOC_TYPE_RELEASE) {
		win32AllocFlags |= MEM_RELEASE;
		size = 0;
	}
	if (win32AllocFlags == 0) {
		return 0;
	}

	int res{ VirtualFree(block, size, win32AllocFlags) };

	return res != 0;
}

void pstd::zeroMemory(void* dst, size_t size) {
	memset(dst, 0, size);
}
void pstd::cpyMemory(void* dst, const void* src, size_t size) {
	memcpy(dst, src, size);
}
void pstd::setMemory(void* dst, int val, size_t size) {
	memset(dst, val, size);
}

pstd::AllocationLimits pstd::getSystemAllocationLimits() {
	SYSTEM_INFO sysInfo{};
	GetSystemInfo(&sysInfo);

	AllocationLimits limits{
		.minAllocSize = sysInfo.dwAllocationGranularity,
		.pageSize = sysInfo.dwPageSize,
	};

	return limits;
}

void pstd::initializeMemorySystem() {
	g_State.allocLimits = getSystemAllocationLimits();
}
