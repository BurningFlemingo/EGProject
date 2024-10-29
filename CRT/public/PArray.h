#pragma once
#include "PAssert.h"
#include "PMemory.h"
#include "PTypes.h"

namespace pstd {
	template<typename T, size_t n = 0>
	struct FixedArray;	// cant grow, doesnt have a count

	template<typename T, size_t n = 0>
	struct BoundedArray;  // cant grow, has a count

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

	template<typename T>
	struct BoundedArray<T, 0> {
		const T& operator[](size_t index) const {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity(allocation) >= count);
			ASSERT(count > index);

			auto* typedBlock{ (const T*)allocation.block };
			const T& val{ typedBlock[index] };
			return val;
		}

		T& operator[](size_t index) {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity(allocation) >= count);
			ASSERT(count > index);

			auto* typedBlock{ (T*)allocation.block };
			T& val{ typedBlock[index] };
			return val;
		}

		Allocation allocation;
		size_t count;
	};

	template<typename T, size_t n>
	struct BoundedArray {
		const T& operator[](size_t index) const {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity(allocation) >= count);
			ASSERT(count > index);

			const T& val{ staticArray[index] };
			return val;
		}

		T& operator[](size_t index) {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity(allocation) >= count);
			ASSERT(count > index);

			T& val{ staticArray[index] };
			return val;
		}
		T staticArray[n];
		Allocation allocation{ .block = staticArray,
							   .size = { n * sizeof(T) } };
		size_t count;
	};

	template<typename T, size_t n>
	constexpr size_t getCapacity(const FixedArray<T, n>& array) {
		size_t capacity{ n };
		return capacity;
	}

	template<typename T, size_t n>
	constexpr size_t getCapacity(const BoundedArray<T, n>& array) {
		size_t capacity{ n };
		return capacity;
	}

	template<typename T>
	size_t getCapacity(const FixedArray<T, 0>& array) {
		ASSERT(array.allocation.block);
		size_t capacity{ array.allocation.size * sizeof(T) };
		return capacity;
	}

	template<typename T>
	size_t getCapacity(const BoundedArray<T, 0>& array) {
		ASSERT(array.allocation.block);
		size_t capacity{ array.allocation.size * sizeof(T) };
		return capacity;
	}

	template<typename T, size_t n>
	void compactRemove(BoundedArray<T, n>* array, size_t index) {
		ASSERT(array);
		ASSERT(array->allocation.block);
		ASSERT(array->count <= pstd::getCapacity(array));
		ASSERT(index < array->count);

		if (array->count <= 1) {
			array->count = 0;
			return;
		}

		size_t lastIndex{ array->count - 1 };
		T lastElement{ array[lastIndex] };
		(*array)[index] = lastElement;
		array->count--;
	}

	template<typename T, size_t n>
	bool find(
		FixedArray<T, n>* array, const T& val, size_t* outIndex = nullptr
	) {
		ASSERT(array);
		ASSERT(out);
		ASSERT(array->allocation.block);

		bool found{};
		size_t foundIndex{};
		for (size_t i{}; i < array->count; i++) {
			if ((*array)[i] == val) {
				found = true;
				foundIndex = i;
				break;
			}
		}

		if (outIndex) {
			*outIndex = foundIndex;
		}
		return found;
	}

	template<typename T, size_t n>
	bool find(
		BoundedArray<T, n>* array, const T& val, size_t* outIndex = nullptr
	) {
		ASSERT(array);
		ASSERT(out);
		ASSERT(array->allocation.block);
		ASSERT(array->count <= pstd::getCapacity(array));

		bool found{};
		size_t foundIndex{};
		for (size_t i{}; i < array->count; i++) {
			if ((*array)[i] == val) {
				found = true;
				foundIndex = i;
				break;
			}
		}

		if (outIndex) {
			*outIndex = foundIndex;
		}
		return found;
	}

}  // namespace pstd
