#pragma once

#include "PTypes.h"
#include "PMemory.h"
#include "PAssert.h"
#include "PAlgorithm.h"

namespace pstd {
	struct Arena {
		Allocation allocation;
		uint32_t offset;

		bool isAllocated;
	};

	struct ArenaFrame {
		Arena* pArena;

		uint32_t* pPersistOffset{ &pArena->offset };
		uint32_t scratchOffset{ ncast<uint32_t>(pArena->allocation.size - 1) };

		bool isFlipped;
	};

	struct ArenaScratchFrame {
		Arena arena;

		uint32_t bottomOffset{};
		uint32_t topOffset{ ncast<uint32_t>(arena.allocation.size - 1) };
		bool isFlipped;
	};

	inline ArenaFrame
		makeFrame(const ArenaFrame& frame, uint32_t* pNewPersistOffset) {
		ASSERT(pNewPersistOffset);

		bool persistOffsetChanged{ pNewPersistOffset != frame.pPersistOffset };
		uint32_t scratchOffset{};
		if (persistOffsetChanged) {
			scratchOffset = *frame.pPersistOffset;
		} else {
			scratchOffset = frame.scratchOffset;
		}

		auto isFlipped{ ncast<bool>(frame.isFlipped ^ persistOffsetChanged) };
		return ArenaFrame{ .pArena = frame.pArena,
						   .pPersistOffset = pNewPersistOffset,
						   .scratchOffset = scratchOffset,
						   .isFlipped = isFlipped };
	}

	inline ArenaFrame makeFrame(Arena* pArena) {
		ASSERT(pArena);

		return ArenaFrame{
			.pArena = pArena,
			.pPersistOffset = &pArena->offset,
			.scratchOffset = ncast<uint32_t>(pArena->allocation.size - 1),
		};
	}

	inline ArenaFrame makeFrame(ArenaScratchFrame* frame) {
		return ArenaFrame{ .pArena = &frame->arena,
						   .pPersistOffset = &frame->bottomOffset,
						   .scratchOffset = frame->topOffset,
						   .isFlipped = false };
	}

	inline ArenaScratchFrame makeScratchFrame(const ArenaFrame& frame) {
		uint32_t bottomOffset{
			min(*frame.pPersistOffset, frame.scratchOffset)
		};
		uint32_t topOffset{ max(*frame.pPersistOffset, frame.scratchOffset) };
		return ArenaScratchFrame{ .arena = *frame.pArena,
								  .bottomOffset = bottomOffset,
								  .topOffset = topOffset,
								  .isFlipped = frame.isFlipped };
	}

	inline ArenaScratchFrame makeScratchFrame(const Arena& arena) {
		return ArenaScratchFrame{
			.arena = arena,
			.bottomOffset = arena.offset,
			.topOffset = ncast<uint32_t>(arena.allocation.size - 1),
			.isFlipped = false
		};
	}

	Arena allocateArena(AllocationRegistry* registry, const size_t size);

	template<typename T>
	Arena allocateArena(AllocationRegistry* registry, const size_t count) {
		size_t byteAllocSize{ count * sizeof(T) };
		return allocateArena(registry, byteAllocSize);
	}

	template<typename T>
	constexpr size_t getCount(const ArenaFrame& frame) {
		if (frame.isFlipped) {
			size_t allocSize{ frame.pArena->allocation.size };
			return (allocSize - *frame.pPersistOffset) / sizeof(T);
		}
		return *frame.pPersistOffset / sizeof(T);
	}
	template<typename T>
	uint32_t getAvailableCount(const ArenaFrame& frame) {
		uintptr_t baseAddress{ (size_t)frame.pArena->allocation.block };
		uint32_t alignment{ alignof(T) };

		uint32_t bottomOffset{
			min(*frame.pPersistOffset, frame.scratchOffset)
		};
		uint32_t topOffset{ max(*frame.pPersistOffset, frame.scratchOffset) };

		uint32_t alignedTopOffset{};
		uint32_t alignedBottomOffset{};

		uint32_t bottomPadding{
			bottomOffset + uint32_t((bottomOffset + alignment) % alignment)
		};

		alignedBottomOffset = bottomOffset + bottomPadding;

		auto topPadding{ ncast<uint32_t>(baseAddress + topOffset) % alignment };

		alignedTopOffset = topOffset + topPadding;

		ASSERT(
			alignedTopOffset >= alignedBottomOffset
		);	// overlapped is free memory

		uint32_t avaliableBytes{ alignedTopOffset - alignedBottomOffset +
								 1 };  // overlapped is free memory
		uint32_t res{ avaliableBytes / sizeof(T) };
		return res;
	}

	void freeArena(AllocationRegistry* registry, Arena* arena);

	Allocation alloc(
		ArenaFrame* arenaFrame, const size_t size, const uint32_t alignment
	);
	inline Allocation alloc(
		ArenaScratchFrame* frame, const size_t size, const uint32_t alignment
	) {
		ASSERT(frame);
		ArenaFrame intermediateFrame{ .pArena = &frame->arena,
									  .pPersistOffset = &frame->bottomOffset,
									  .scratchOffset = frame->topOffset,
									  .isFlipped = frame->isFlipped };
		Allocation allocation{ alloc(&intermediateFrame, size, alignment) };
		frame->topOffset =
			intermediateFrame.scratchOffset;  // only offset passed by value
		return allocation;
	}

	Allocation scratchAlloc(
		ArenaFrame* frame, const size_t size, const uint32_t alignment
	);

	inline Allocation scratchAlloc(
		ArenaScratchFrame* frame, const size_t size, const uint32_t alignment
	) {
		ASSERT(frame);
		ArenaFrame intermediateFrame{ .pArena = &frame->arena,
									  .pPersistOffset = &frame->bottomOffset,
									  .scratchOffset = frame->topOffset,
									  .isFlipped = frame->isFlipped };
		Allocation allocation{
			scratchAlloc(&intermediateFrame, size, alignment)
		};
		frame->topOffset =
			intermediateFrame.scratchOffset;  // only offset passed by value
		return allocation;
	}

	template<typename T>
	Allocation alloc(ArenaFrame* frame, const size_t count = 1) {
		ASSERT(frame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ alloc(frame, allocSize, alignof(T)) };
	}

	template<typename T>
	Allocation alloc(ArenaScratchFrame* frame, const size_t count = 1) {
		ASSERT(frame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ alloc(frame, allocSize, alignof(T)) };
	}

	template<typename T>
	Allocation scratchAlloc(ArenaFrame* frame, const size_t count = 1) {
		ASSERT(frame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ scratchAlloc(frame, allocSize, alignof(T)) };
	}
	template<typename T>
	Allocation scratchAlloc(ArenaScratchFrame* frame, const size_t count = 1) {
		ASSERT(frame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ scratchAlloc(frame, allocSize, alignof(T)) };
	}

	template<typename T>
	void* getAlignedOffset(const ArenaFrame& arenaFrame) {
		return getAlignedOffset(arenaFrame, alignof(T));
	}

	template<typename T>
	void* getAlignedScratchOffset(const ArenaFrame& arenaFrame) {
		return getAlignedScratchOffset(arenaFrame, alignof(T));
	}

	Allocation makeShallowCopy(
		ArenaFrame&& arenaFrame, const Allocation& b, uint32_t alignment
	);

	template<typename T>
	Allocation makeShallowCopy(ArenaFrame&& arenaFrame, const Allocation& b) {
		return makeShallowCopy(pstd::move(arenaFrame), b, alignof(T));
	}

	Allocation concat(
		ArenaFrame&& arenaFrame,
		const Allocation& a,
		const Allocation& b,
		uint32_t alignment
	);

	template<typename T>
	Allocation
		concat(ArenaFrame&& frame, const Allocation& a, const Allocation& b) {
		return concat(pstd::move(frame), a, b, alignof(T));
	}

	inline void reset(Arena* arena) {
		ASSERT(arena);

		arena->offset = 0;
	}
}  // namespace pstd
