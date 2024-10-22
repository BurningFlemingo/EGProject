#pragma once
#include "Base.h"

struct FixedArena {
	void* block;
	size_t size;
	size_t headOffset;
};

FixedArena createFixedArena(const size_t size, void* baseAddress = nullptr);

template<typename T>
FixedArena createFixedArena(const size_t count, void* baseAddress = nullptr) {
	const size_t bytesToAllocate{ count * sizeof(T) };
	createFixedArena(bytesToAllocate, baseAddress);
}
void* fixedArenaAlloc(
	FixedArena* arena, const size_t size, const uint32_t alignment
);

template<typename T>
T* fixedArenaAlloc(FixedArena* arena, const size_t count) {
	uint32_t alignment{ sizeof(T) };
	T* block{ static_cast<T*>(fixedArenaAlloc(arena, count, alignment)) };
	return block;
}

void destroyFixedArena(FixedArena* arena);

inline void resetArena(FixedArena* arena) {
	arena->headOffset = 0;
}
