#pragma once

#include "PTypes.h"
#include "PMemory.h"
#include "PAssert.h"

namespace pstd {
	// has 2 regions that grow towards eachother
	struct FixedArena {
		const Allocation allocation;
		size_t bottomOffset;  // grows up
		size_t topOffset{ allocation.size - 1 };  // grows down

		bool isAllocated;
	};

	struct FixedArenaFrameState {
		size_t scratchOffset;
		bool isFlipped{ false };
	};

	// contains a flippable persistant and a scratch region in which only one
	// persists after the frame's scope has ended (assuming pArena has a longer
	// lifetime than the fixed arena frame)
	struct FixedArenaFrame {
		FixedArena* pArena;
		FixedArenaFrameState state{ .scratchOffset = pArena->topOffset };
	};

	// arena is stored directly to not change the original. Functions using this
	// only need scratch space (no return space)
	struct FixedScratchArenaFrame {
		FixedArena arena;
		FixedArenaFrameState state{ .scratchOffset = arena.topOffset };
	};

	inline size_t getPersistantOffset(const FixedArenaFrame& frame) {
		if (frame.state.isFlipped) {
			return frame.pArena->topOffset;
		}
		return frame.pArena->bottomOffset;
	}
	inline size_t getPersistantOffset(const FixedScratchArenaFrame& frame) {
		if (frame.state.isFlipped) {
			return frame.arena.topOffset;
		}
		return frame.arena.bottomOffset;
	}

	inline FixedArenaFrame makeFlipped(FixedArenaFrame&& frame) {
		frame.state.isFlipped = !frame.state.isFlipped;
		return FixedArenaFrame{ .pArena = frame.pArena, .state = frame.state };
	}
	inline FixedScratchArenaFrame makeFlipped(FixedScratchArenaFrame&& frame) {
		frame.state.isFlipped = !frame.state.isFlipped;
		return FixedScratchArenaFrame{ .arena = frame.arena,
									   .state = frame.state };
	}

	FixedArena
		allocateFixedArena(AllocationRegistry* registry, const size_t size);

	template<typename T>
	FixedArena
		allocateFixedArena(AllocationRegistry* registry, const size_t count) {
		size_t byteAllocSize{ count * sizeof(T) };
		return FixedArena{ allocateFixedArena(registry, byteAllocSize) };
	}

	template<typename T>
	constexpr size_t getCount(const FixedArenaFrame& frame) {
		if (frame.state.isFlipped) {
			size_t allocSize{ frame.pArena->allocation.size };
			return (allocSize - frame.state.stackOffset) / sizeof(T);
		}
		return frame.pArena->bottomOffset / sizeof(T);
	}
	template<typename T>
	size_t getAvailableCount(const FixedArenaFrame& frame) {
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

	void freeFixedArena(AllocationRegistry* registry, FixedArena* arena);

	Allocation alloc(
		FixedArenaFrame* arenaFrame, const size_t size, const uint32_t alignment
	);
	inline Allocation alloc(
		FixedScratchArenaFrame* frame,
		const size_t size,
		const uint32_t alignment
	) {
		ASSERT(frame);
		FixedArenaFrame intermediateFrame{ .pArena = &frame->arena,
										   .state = frame->state };
		Allocation allocation{ alloc(&intermediateFrame, size, alignment) };
		frame->state = intermediateFrame.state;
		return allocation;
	}

	Allocation scratchAlloc(
		FixedArenaFrame* frame, const size_t size, const uint32_t alignment
	);

	inline Allocation scratchAlloc(
		FixedScratchArenaFrame* frame,
		const size_t size,
		const uint32_t alignment
	) {
		ASSERT(frame);
		FixedArenaFrame intermediateFrame{ .pArena = &frame->arena,
										   .state = frame->state };
		Allocation allocation{
			scratchAlloc(&intermediateFrame, size, alignment)
		};
		frame->state = intermediateFrame.state;
		return allocation;
	}

	template<typename T>
	Allocation alloc(FixedArenaFrame* frame, const size_t count = 1) {
		ASSERT(frame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ alloc(frame, allocSize, alignof(T)) };
	}

	template<typename T>
	Allocation alloc(FixedScratchArenaFrame* frame, const size_t count = 1) {
		ASSERT(frame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ alloc(frame, allocSize, alignof(T)) };
	}

	template<typename T>
	Allocation scratchAlloc(FixedArenaFrame* frame, const size_t count = 1) {
		ASSERT(frame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ scratchAlloc(frame, allocSize, alignof(T)) };
	}
	template<typename T>
	Allocation
		scratchAlloc(FixedScratchArenaFrame* frame, const size_t count = 1) {
		ASSERT(frame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ scratchAlloc(frame, allocSize, alignof(T)) };
	}

	// padding is added to offset
	void*
		getAlignedOffset(const FixedArenaFrame& arenaFrame, uint32_t alignment);

	// padding is subtracted to offset
	void* getAlignedScratchOffset(
		const FixedArenaFrame& arenaFrame, uint32_t alignment
	);

	template<typename T>
	void* getAlignedOffset(const FixedArenaFrame& arenaFrame) {
		return getAlignedOffset(arenaFrame, alignof(T));
	}

	template<typename T>
	void* getAlignedScratchOffset(const FixedArenaFrame& arenaFrame) {
		return getAlignedScratchOffset(arenaFrame, alignof(T));
	}

	inline void reset(FixedArena* arena) {
		ASSERT(arena);

		arena->bottomOffset = 0;
		arena->topOffset = arena->allocation.size - 1;
	}
	inline void resetScratch(FixedArena* arena) {
		ASSERT(arena);

		arena->topOffset = arena->allocation.size - 1;
	}
}  // namespace pstd
