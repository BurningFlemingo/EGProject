#pragma once

#include "PTypes.h"
#include "PMemory.h"
#include "PAssert.h"

namespace pstd {
	// has 2 regions that grow towards eachother
	struct Arena {
		const Allocation allocation;
		uint32_t bottomOffset;	// grows up
		uint32_t topOffset{ allocation.size - 1 };	// grows down

		bool isAllocated;
	};

	struct ArenaFrameState {
		uint32_t scratchOffset;
		bool isFlipped{ false };
	};

	// used to control function return placement within pArena.
	// Also limits scratch allocations lifetime to the frame's scope. returns
	// live in pArena.
	struct ArenaFrame {
		Arena* pArena;
		ArenaFrameState state{ .scratchOffset = pArena->topOffset };
	};

	// uses a copy of an arena for scratch allocations, returns dont live here.
	struct ScratchArenaFrame {
		Arena arena;
		ArenaFrameState state{ .scratchOffset = arena.topOffset };
	};

	// the return should be used for function passing as an r-value.
	// makes alloc -> scratchAlloc and scratchAlloc -> alloc. note, scratchAlloc
	// allocations dont persist after ArenaFrame's scope ends
	inline ArenaFrame makeFlipped(ArenaFrame&& frame) {
		frame.state.isFlipped = !frame.state.isFlipped;
		return ArenaFrame{ .pArena = frame.pArena, .state = frame.state };
	}

	Arena allocateArena(AllocationRegistry* registry, const size_t size);

	template<typename T>
	Arena allocateArena(AllocationRegistry* registry, const size_t count) {
		size_t byteAllocSize{ count * sizeof(T) };
		return Arena{ allocateFixedArena(registry, byteAllocSize) };
	}

	template<typename T>
	constexpr size_t getCount(const ArenaFrame& frame) {
		if (frame.state.isFlipped) {
			size_t allocSize{ frame.pArena->allocation.size };
			return (allocSize - frame.state.scratchOffset) / sizeof(T);
		}
		return frame.pArena->bottomOffset / sizeof(T);
	}
	template<typename T>
	size_t getAvailableCount(const ArenaFrame& frame) {
		size_t baseAddress{ (size_t)frame.pArena->allocation.block };
		uint32_t alignment{ alignof(T) };

		size_t alignedTopOffset{};
		size_t alignedBottomOffset{};

		if (frame.state.isFlipped) {
			size_t topOffset{ frame.pArena->allocation.size };
			size_t bottomOffset{ frame.pArena->topOffset };

			size_t bottomPadding{ bottomOffset +
								  ((bottomOffset + alignment) % alignment) };

			size_t topPadding{ (baseAddress + topOffset) % alignment };

			alignedBottomOffset = bottomOffset + bottomPadding;
			alignedTopOffset = topOffset - topPadding;

		} else {
			alignedBottomOffset =
				0;	// if this wasnt true, alloc would have asserted

			size_t topOffset{ frame.state.scratchOffset };
			size_t topPadding{ (baseAddress + topOffset) % alignment };

			alignedTopOffset = topOffset + topPadding;
		}

		ASSERT(
			alignedTopOffset >= alignedBottomOffset
		);	// overlapped is free memory

		size_t avaliableBytes{ alignedTopOffset - alignedBottomOffset +
							   1 };	 // overlapped is free memory
		size_t res{ avaliableBytes / sizeof(T) };
		return res;
	}

	void freeArena(AllocationRegistry* registry, Arena* arena);

	Allocation alloc(
		ArenaFrame* arenaFrame, const size_t size, const uint32_t alignment
	);
	inline Allocation alloc(
		ScratchArenaFrame* frame, const size_t size, const uint32_t alignment
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
		ScratchArenaFrame* frame, const size_t size, const uint32_t alignment
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
