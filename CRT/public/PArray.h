#pragma once
#include "PAssert.h"
#include "PMemory.h"
#include "PTypes.h"

namespace pstd {
	template<typename T, size_t n = 0>
	struct FixedArray;

	template<typename T>
	struct FixedArray<T, 0> {
		const T& operator[](size_t index) const {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity(allocation) > index);

			auto* typedBlock{ (const T*)allocation.block };
			const T& val{ typedBlock[index] };
			return val;
		}

		T& operator[](size_t index) {
			ASSERT(allocation.block);
			ASSERT(getCapacity(allocation) > index);

			auto* typedBlock{ (T*)allocation.block };
			T& val{ typedBlock[index] };
			return val;
		}

		Allocation allocation;
	};

	template<typename T, size_t n>
	struct FixedArray {
		const T& operator[](size_t index) const {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity(allocation) > index);

			const T& val{ staticArray[index] };
			return val;
		}

		T& operator[](size_t index) {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity(allocation) > index);

			T& val{ staticArray[index] };
			return val;
		}
		T staticArray[n];
		Allocation allocation{ .block = staticArray,
							   .size = { n * sizeof(T) } };
	};

	template<typename T, size_t n>
	constexpr size_t getCapacity(const FixedArray<T, n>& array) {
		size_t capacity{ n };
		return capacity;
	}

	template<typename T>
	size_t getCapacity(const FixedArray<T, 0>& array) {
		ASSERT(array.allocation.block);
		size_t capacity{ array.allocation.size * sizeof(T) };
		return capacity;
	}
}  // namespace pstd
