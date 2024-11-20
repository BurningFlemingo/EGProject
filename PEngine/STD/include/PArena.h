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

		uint32_t bottomOffset{ ncast<uint32_t>(arena.allocation.size - 1) };
		uint32_t topOffset{ ncast<uint32_t>(arena.allocation.size - 1) };
		bool isFlipped;
	};

	inline ArenaFrame
		makeFrame(const ArenaFrame& frame, uint32_t* pNewPersistOffset) {
		bool isFlipped{ pNewPersistOffset != frame.pPersistOffset };
		uint32_t scratchOffset{};
		if (isFlipped) {
			scratchOffset = *frame.pPersistOffset;
		} else {
			scratchOffset = frame.scratchOffset;
		}
		return ArenaFrame{ .pArena = frame.pArena,
						   .pPersistOffset = pNewPersistOffset,
						   .scratchOffset = scratchOffset,
						   .isFlipped = isFlipped };
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

	Arena allocateArena(AllocationRegistry* registry, const size_t size);

	template<typename T>
	Arena allocateArena(AllocationRegistry* registry, const size_t count) {
		size_t byteAllocSize{ count * sizeof(T) };
		return allocateArena(registry, byteAllocSize);
	}

	// template<typename T>
	// constexpr size_t getCount(const ArenaFrame& frame) {
	// 	if (frame.isFlipped) {
	// 		size_t allocSize{ frame.pArena->allocation.size };
	// 		return (allocSize - frame.scratchOffset) / sizeof(T);
	// 	}
	// 	return *frame.pPersistOffset / sizeof(T);
	// }
	// template<typename T>
	// uint32_t getAvailableCount(const ArenaFrame& frame) {
	// 	uintptr_t baseAddress{ (size_t)frame.pArena->allocation.block };
	// 	uint32_t alignment{ alignof(T) };

	// 	uint32_t alignedTopOffset{};
	// 	uint32_t alignedBottomOffset{};

	// 	if (frame.state.isFlipped) {
	// 		uint32_t topOffset{ ncast<uint32_t>(frame.pArena->allocation.size
	// 		) };
	// 		uint32_t bottomOffset{ frame.pArena->topOffset };

	// 		uint32_t bottomPadding{
	// 			bottomOffset + uint32_t((bottomOffset + alignment) % alignment)
	// 		};

	// 		auto topPadding{ ncast<uint32_t>(baseAddress + topOffset) %
	// 						 alignment };

	// 		alignedBottomOffset = bottomOffset + bottomPadding;
	// 		alignedTopOffset = topOffset - topPadding;

	// 	} else {
	// 		alignedBottomOffset =
	// 			0;	// if this wasnt true, alloc would have asserted

	// 		uint32_t topOffset{ frame.state.scratchOffset };
	// 		auto topPadding{ ncast<uint32_t>(baseAddress + topOffset) %
	// 						 alignment };

	// 		alignedTopOffset = topOffset + topPadding;
	// 	}

	// 	ASSERT(
	// 		alignedTopOffset >= alignedBottomOffset
	// 	);	// overlapped is free memory

	// 	uint32_t avaliableBytes{ alignedTopOffset - alignedBottomOffset +
	// 							 1 };  // overlapped is free memory
	// 	uint32_t res{ avaliableBytes / sizeof(T) };
	// 	return res;
	// }

	void freeArena(AllocationRegistry* registry, Arena* arena);

	Allocation alloc(
		ArenaFrame* arenaFrame, const size_t size, const uint32_t alignment
	);
	inline Allocation alloc(
		ArenaScratchFrame* frame, const size_t size, const uint32_t alignment
	) {
		ASSERT(frame);
		ArenaFrame intermediateFrame{ .pArena = &frame->arena,
									  .state = frame->state };
		Allocation allocation{ alloc(&intermediateFrame, size, alignment) };
		frame->state = intermediateFrame.state;
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
									  .state = frame->state };
		Allocation allocation{
			scratchAlloc(&intermediateFrame, size, alignment)
		};
		frame->state = intermediateFrame.state;
		return allocation;
	}

	template<typename T>
	Allocation alloc(ArenaFrame* frame, const size_t count = 1) {
		ASSERT(frame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ alloc(frame, allocSize, alignof(T)) };
	}

	template<typename T>
	Allocation alloc(ScratchArenaFrame* frame, const size_t count = 1) {
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
	Allocation scratchAlloc(ScratchArenaFrame* frame, const size_t count = 1) {
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

	inline void reset(Arena* arena) {
		ASSERT(arena);

		arena->bottomOffset = 0;
		arena->topOffset = arena->allocation.size - 1;
	}
	inline void resetScratch(Arena* arena) {
		ASSERT(arena);

		arena->topOffset = arena->allocation.size - 1;
	}
}  // namespace pstd
