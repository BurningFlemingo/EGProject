#pragma once
#include "PAssert.h"
#include "PMemory.h"
#include <stdint.h>

namespace pstd {
	template<typename T>
	struct FixedArray {
		Allocation allocation;
		size_t count;
	};
	template<typename T>
	constexpr size_t getCapacity(const FixedArray<T>& buffer) {
		size_t capacity{ buffer.allocation.size / sizeof(T) };
		return capacity;
	}

	template<typename T>
	bool isFull(const FixedArray<T>& buffer) {
		bool res{};
		if (getElementCount(buffer) == getCapacity(buffer)) {
			res = true;
		}
		return res;
	}

	template<typename T>
	bool isEmpty(const FixedArray<T>& buffer) {
		bool res{};
		if (buffer.headIndex == buffer.tailIndex) {
			res = true;
		}
		return res;
	}

	template<typename T>
	void indexWrite(FixedArray<T>* buffer, const size_t index, const T val) {
		ASSERT(buffer);
		ASSERT(getCapacity(buffer) > index);

		T* typedBlock{ (T*)buffer->allocation.block };
		typedBlock[index] = val;
	}

	template<typename T>
	T indexRead(const FixedArray<T>& buffer, const size_t index) {
		ASSERT(buffer);
		ASSERT(getCapacity(buffer) > index);

		const T* typedBlock{ (const T*)buffer.allocation.block };
		T val{ typedBlock[index] };
		return val;
	}
}  // namespace pstd
