#pragma once
#include "PMemory.h"
#include "PAlgorithm.h"
#include "PAssert.h"
#include "PArray.h"
#include "PContainer.h"

namespace pstd {
	template<typename T>
	struct CircularBuffer {
		using ElementType = T;

		Allocation allocation;
		size_t headIndex;
		size_t tailIndex;
	};

	template<typename T>
	size_t getCount(const CircularBuffer<T>& buffer) {
		size_t res{};
		if (buffer.headIndex >= buffer.tailIndex) {
			res = buffer.headIndex - buffer.tailIndex;
		} else {
			size_t capacity{ getCapacity(buffer) };
			res = capacity - (buffer.tailIndex - buffer.headIndex);
		}

		return res;
	}

	template<typename T>
	Array<T> getContents(const CircularBuffer<T>& buffer) {
		size_t address{ (size_t)buffer.allocation.block + buffer.tailIndex };
		size_t count{ getCount(buffer) };
		pstd::Allocation bufferAllocation{};
		return pstd::Array<T>{ .allocation = { .block = (void*)address,
											   .size = count * sizeof(T) } };
	}

	template<typename T>
	bool isFull(const CircularBuffer<T>& buffer) {
		return getCount(buffer) >= getCapacity(buffer);
	}

	template<typename T>
	bool isEmpty(const CircularBuffer<T>& buffer) {
		return buffer.headIndex == buffer.tailIndex;
	}

	template<typename T>
	bool popBack(CircularBuffer<T>* buffer, T* popOut) {
		ASSERT(buffer);
		ASSERT(popOut);

		if (pstd::isEmpty(*buffer)) {
			return false;
		}

		T* const typedBlock{ (T*)buffer->allocation.block };

		buffer->headIndex--;  // headIndex points to the next avaliable index
		*popOut = typedBlock[buffer->headIndex];

		return true;
	}

	template<typename T>
	void pushBackOverwrite(CircularBuffer<T>* buffer, const T val) {
		ASSERT(buffer);

		T* const typedBlock{ (T*)buffer->allocation.block };
		typedBlock[buffer->headIndex] = val;

		buffer->headIndex = (buffer->headIndex + 1) % (getCapacity(*buffer));

		if (isFull(*buffer)) {
			buffer->tailIndex =
				(buffer->tailIndex + 1) % (getCapacity(*buffer));
		}
	};

}  // namespace pstd
