#include "PMemory.h"
#include <Windows.h>

using namespace pstd;

pstd::Allocation pstd::allocMemory(
	const size_t size, AllocationTypeFlagBits allocTypeFlags, void* baseAddress
) {
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

	void* block{
		VirtualAlloc(baseAddress, size, win32AllocFlags, win32SecurityFlags)
	};

	Allocation allocation{ .block = block, .size = size };

	return allocation;
}

bool pstd::freeMemory(
	void* block, AllocationTypeFlagBits allocTypeFlags, size_t size
) {
	uint32_t win32AllocFlags{};
	if (allocTypeFlags & (ALLOC_TYPE_RESERVE | ALLOC_TYPE_COMMIT)) {
		return 0;
	}
	if (allocTypeFlags & ALLOC_TYPE_RESERVE) {
		win32AllocFlags |= MEM_RELEASE;
		size = 0;
	}
	if (allocTypeFlags & ALLOC_TYPE_COMMIT) {
		win32AllocFlags |= MEM_DECOMMIT;
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
