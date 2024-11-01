#include "public/PMemory.h"
#include "public/PMemory.h"

#include "PAssert.h"
#include "PAlgorithm.h"
#include "PArena.h"

void pstd::memSet(void* dst, int val, size_t size) {
	memset(dst, val, size);
}
void pstd::memZero(void* dst, size_t size) {
	memset(dst, 0, size);
}
void pstd::memCpy(void* dst, const void* src, size_t size) {
	memcpy(dst, src, size);
}

void pstd::memMov(void* dst, const void* src, size_t size) {
	ASSERT(dst);
	ASSERT(src);

	auto dstBlock{ (char*)dst };
	auto srcBlock = (const char*)src;

	if (dstBlock < srcBlock) {
		while (size) {
			*dstBlock = *srcBlock;
			dstBlock++;
			srcBlock++;
			size--;
		}
		return;
	}

	dstBlock += size;
	srcBlock += size;
	while (size) {
		*dstBlock = *srcBlock;
		dstBlock--;
		srcBlock--;
		size--;
	}
}

uint32_t pstd::calcAddressAlignmentPadding(
	const void* address, const uint32_t alignment
) {
	uint32_t bytesUnaligned{ (uint32_t)((size_t)address % alignment) };
	// the last % alignment is to set the bytes required to align to zero if
	// the address is already aligned
	return (alignment - bytesUnaligned) % alignment;
}

void pstd::shallowCopy(Allocation* dst, const Allocation& src) {
	ASSERT(dst);
	ASSERT(dst->block);
	ASSERT(src.block);

	size_t copySize{ min(src.size, dst->size) };
	memCpy(dst->block, src.block, copySize);
}

void pstd::shallowMove(Allocation* dst, const Allocation& src) {
	ASSERT(dst);
	ASSERT(dst->block);
	ASSERT(src.block);

	size_t moveSize{ min(src.size, dst->size) };
	memMov(dst->block, src.block, moveSize);
}

pstd::Allocation pstd::concat(
	FixedArena* arena,
	const Allocation& a,
	const Allocation& b,
	uint32_t alignment
) {
	ASSERT(arena);
	ASSERT(a.block);
	ASSERT(b.block);
	size_t allocSize{ a.size + b.size };

	ASSERT(allocSize >= a.size);  // overflow check

	Allocation allocation{ pstd::arenaAlloc(arena, allocSize, alignment) };

	pstd::shallowMove(&allocation, a);

	Allocation headAllocation{ allocation };
	headAllocation.block = (void*)((size_t)headAllocation.block + a.size);
	headAllocation.size -= a.size;

	pstd::shallowMove(&headAllocation, b);

	return allocation;
}
