#pragma once
#include "PMemory.h"
#include "PAlgorithm.h"
#include "PAssert.h"
#include "PArray.h"

namespace pstd {
	template<typename T>
	struct CircularBuffer {
		Allocation allocation;
		size_t headIndex;
		size_t tailIndex;
	};

	template<typename T>
	constexpr size_t getCapacity(const CircularBuffer<T>& buffer) {
		size_t capacity{ buffer.allocation.size / sizeof(T) };
		return capacity;
	}

	template<typename T>
	size_t getCount(const CircularBuffer<T>& buffer) {
		size_t occupancy{};
		if (buffer.headIndex > buffer.tailIndex) {
			occupancy = buffer.headIndex - buffer.tailIndex;
		} else {
			size_t capacity{ getCapacity(buffer) };
			occupancy = capacity - (buffer.tailIndex - buffer.headIndex);
		}
		return occupancy;
	}

	template<typename T>
	FixedArray<T> getContents(const CircularBuffer<T>& buffer) {
		size_t address{ (size_t)buffer.allocation.block + buffer.tailIndex };
		size_t count{ getCount(buffer) };
		pstd::Allocation bufferAllocation{};
		pstd::FixedArray<T> contentsArray{
			.allocation = { .block = (void*)address, .size = count * sizeof(T) }
		};
		return contentsArray;
	}

	template<typename T>
	bool isFull(const CircularBuffer<T>& buffer) {
		bool res{};
		if (getElementCount(buffer) == getCapacity(buffer)) {
			res = true;
		}
		return res;
	}

	template<typename T>
	bool isEmpty(const CircularBuffer<T>& buffer) {
		bool res{};
		if (buffer.headIndex == buffer.tailIndex) {
			res = true;
		}
		return res;
	}

	template<typename T>
	void
		indexWrite(CircularBuffer<T>* buffer, const size_t index, const T val) {
		ASSERT(buffer);
		ASSERT(getCapacity(buffer) > index);

		T* typedBlock{ (T*)buffer->allocation.block };
		typedBlock[index] = val;
	}

	template<typename T>
	T indexRead(const CircularBuffer<T>& buffer, const size_t index) {
		ASSERT(buffer);
		ASSERT(getCapacity(buffer) > index);

		const T* typedBlock{ (const T*)buffer.allocation.block };
		T val{ typedBlock[index] };
		return val;
	}

	template<typename T>
	void pushBack(CircularBuffer<T>* buffer, const T val) {
		ASSERT(buffer);
		ASSERT(!isFull(*buffer));

		T* const typedBlock{ (T*)buffer->allocation.block };
		typedBlock[buffer->headIndex] = val;

		buffer->headIndex = (buffer->headIndex + 1) % (getCapacity(*buffer));
	};

}  // namespace pstd
