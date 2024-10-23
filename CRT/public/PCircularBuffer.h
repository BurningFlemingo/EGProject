#pragma once
#include "PMemory.h"

namespace pstd {
	template<typename T>
	struct CircularBuffer {
		Allocation allocation;
		size_t headOffset;
		size_t tailOffset;
	};

	// template<typename T>
	// CircularBuffer<T>
	// 	allocateCircularBuffer(const size_t count, void* baseAddress = nullptr);

	template<typename T>
	void
		indexWrite(CircularBuffer<T>* buffer, const size_t index, const T val) {
		size_t count{ buffer->allocation.size / sizeof(T) };
		if (index < count) {
			T* typedBlock{ (T*)buffer->allocation.block };
			typedBlock[index] = val;
		}
	}

	template<typename T>
	T indexRead(const CircularBuffer<T>& buffer, const size_t index) {
		size_t count{ buffer.allocation.size / sizeof(T) };
		T val{};
		if (index < count) {
			T* typedBlock{ (T*)buffer.allocation.block };
			val = typedBlock[index];
		}
		return val;
	}

	template<typename T>
	void pushBack(CircularBuffer<T>* buffer, const T val) {
		T* typedBlock{ (T*)buffer->allocation.block };
		typedBlock[buffer->headOffset] = val;

		buffer->headOffset++;

		buffer->headOffset =
			buffer->headOffset % (buffer->allocation.size / sizeof(T));
	};

	// circularBuffer.allocation

}  // namespace pstd
