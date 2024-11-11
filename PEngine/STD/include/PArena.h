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

	struct StackFrame {
		size_t stackOffset;
		bool isFlipped{ false };
	};

	// contains a flippable persistant and a temporary region in which only one
	// persists after the frame's scope has ended.
	struct FixedArenaFrame {
		FixedArena* pArena;

		StackFrame frame{ .stackOffset = pArena->topOffset };
	};
	// arena is stored directly to not change the original. Functions using this
	// only need scratch space (no return space)
	struct FixedScratchArenaFrame {
		FixedArena arena;
		StackFrame frame{ .stackOffset = arena.topOffset };
	};

	inline FixedArenaFrame makeFlipped(FixedArenaFrame&& arenaFrame) {
		auto& [arena, frame] = arenaFrame;

		frame.isFlipped = !frame.isFlipped;
		return FixedArenaFrame{ .pArena = arena, .frame = frame };
	}
	inline FixedScratchArenaFrame
		makeFlipped(FixedScratchArenaFrame&& arenaFrame) {
		auto& [arena, frame] = arenaFrame;
		frame.isFlipped = !frame.isFlipped;
		return FixedScratchArenaFrame{ .arena = arena, .frame = frame };
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
	constexpr size_t getCount(const FixedArenaFrame& arenaFrame) {
		const auto& [arena, frame] = arenaFrame;
		if (frame.isFlipped) {
			return frame.stackOffset / sizeof(T);
		}
		return arena->bottomOffset / sizeof(T);
	}
	template<typename T>
	size_t getAvailableCount(const FixedArenaFrame& arenaFrame) {
		const auto& [arena, frame] = arenaFrame;
		size_t bottomOffset{};
		size_t topOffset{};
		size_t baseAddress{ (size_t)arena->allocation.block };
		if (frame.isFlipped) {
			bottomOffset = frame.stackOffset;
			topOffset = arena->bottomOffset;
		} else {
			bottomOffset = arena->topOffset;
			topOffset = frame.stackOffset;
		}

		size_t bottomPadding{
			pstd::calcAddressAlignmentPadding<T>(baseAddress + bottomOffset)
		};

		uint32_t alignment{ alignof(T) };
		size_t topPadding{ (baseAddress + topOffset) % alignment };

		size_t alignedBottomOffset{ bottomOffset + bottomPadding };
		size_t alignedTopOffset{
			topOffset - topPadding
		};	// increases in reverse direction

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
		FixedScratchArenaFrame* scratchArenaFrame,
		const size_t size,
		const uint32_t alignment
	) {
		ASSERT(scratchArenaFrame);
		FixedArenaFrame intermediateFrame{ .pArena = &scratchArenaFrame->arena,
										   .frame = scratchArenaFrame->frame };
		Allocation allocation{ alloc(&intermediateFrame, size, alignment) };
		scratchArenaFrame->frame = intermediateFrame.frame;
		return allocation;
	}

	Allocation scratchAlloc(
		FixedArenaFrame* arenaFrame, const size_t size, const uint32_t alignment
	);

	inline Allocation scratchAlloc(
		FixedScratchArenaFrame* scratchArenaFrame,
		const size_t size,
		const uint32_t alignment
	) {
		ASSERT(scratchArenaFrame);
		FixedArenaFrame intermediateFrame{ .pArena = &scratchArenaFrame->arena,
										   .frame = scratchArenaFrame->frame };
		Allocation allocation{
			scratchAlloc(&intermediateFrame, size, alignment)
		};
		scratchArenaFrame->frame = intermediateFrame.frame;
		return allocation;
	}

	template<typename T>
	Allocation alloc(FixedArenaFrame* arenaFrame, const size_t count = 1) {
		ASSERT(arenaFrame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ alloc(arenaFrame, allocSize, alignof(T)) };
	}

	template<typename T>
	Allocation alloc(
		FixedScratchArenaFrame* scratchArenaFrame, const size_t count = 1
	) {
		ASSERT(scratchArenaFrame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ alloc(scratchArenaFrame, allocSize, alignof(T)) };
	}

	template<typename T>
	Allocation
		scratchAlloc(FixedArenaFrame* arenaFrame, const size_t count = 1) {
		ASSERT(arenaFrame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{ scratchAlloc(arenaFrame, allocSize, alignof(T)) };
	}
	template<typename T>
	Allocation scratchAlloc(
		FixedScratchArenaFrame* scratchArenaFrame, const size_t count = 1
	) {
		ASSERT(scratchArenaFrame);

		size_t allocSize{ count * sizeof(T) };
		return Allocation{
			scratchAlloc(scratchArenaFrame, allocSize, alignof(T))
		};
	}

	void* getNextAllocAddress(
		const FixedArenaFrame& arenaFrame, uint32_t alignment
	);

	void* getNextScratchAllocAddress(
		const FixedArenaFrame& arenaFrame, uint32_t alignment
	);

	template<typename T>
	void* getNextAllocAddress(const FixedArenaFrame& arenaFrame) {
		return getNextAllocAddress(arenaFrame, alignof(T));
	}

	template<typename T>
	void* getNextScratchAllocAddress(const FixedArenaFrame& arenaFrame) {
		return getNextScratchAllocAddress(arenaFrame, alignof(T));
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
