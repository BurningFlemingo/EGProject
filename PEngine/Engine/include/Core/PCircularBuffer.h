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

		T* block;
		size_t size;
		size_t headIndex;
		size_t tailIndex;
	};

	template<typename T>
	size_t getCapacity(const CircularBuffer<T>& buffer) {
		return buffer.size / sizeof(T);
	}

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
	bool isFull(const CircularBuffer<T>& buffer) {
		return getCount(buffer) >= getCapacity(buffer);
	}

	template<typename T>
	bool isEmpty(const CircularBuffer<T>& buffer) {
		return buffer.headIndex == buffer.tailIndex;
	}

	template<typename T>
	bool popBack(CircularBuffer<T>* buffer, T* popOut = nullptr) {
		ASSERT(buffer);
		ASSERT(buffer->headIndex < getCapacity(*buffer));

		if (pstd::isEmpty(*buffer)) {
			return false;
		}

		if (buffer->headIndex == 0) {
			buffer->headIndex = getCapacity(*buffer) - 1;
		} else {
			buffer->headIndex--;
		}

		if (popOut != nullptr) {
			*popOut = buffer->block[buffer->headIndex];
		}

		return true;
	}

	template<typename T>
	void pushBackOverwrite(CircularBuffer<T>* buffer, const T val) {
		ASSERT(buffer);

		bool bufferWasFull{ isFull(*buffer) };

		buffer->block[buffer->headIndex] = val;

		buffer->headIndex = (buffer->headIndex + 1) % (getCapacity(*buffer));

		if (bufferWasFull) {
			buffer->tailIndex =
				(buffer->tailIndex + 1) % (getCapacity(*buffer));
		}
	};

}  // namespace pstd
