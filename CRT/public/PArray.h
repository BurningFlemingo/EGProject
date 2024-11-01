#pragma once
#include "PAssert.h"
#include "PMemory.h"
#include "PTypes.h"
#include "PArena.h"
#include "PContainer.h"

namespace pstd {
	template<typename T, size_t n = 0>
	struct FixedArray;	// cant grow, doesnt have a count

	template<typename T, size_t n = 0>
	struct BoundedArray;  // cant grow, has a count

	template<typename T>
	struct FixedArray<T, 0> {
		using ElementType = T;

		const T& operator[](size_t index) const {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity<T>(allocation) > index);

			return ((const T*)allocation.block)[index];
		}

		T& operator[](size_t index) {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity<T>(allocation) > index);

			return ((T*)allocation.block)[index];
		}

		Allocation allocation;
	};

	template<typename T, size_t n>
	struct FixedArray {
		using ElementType = T;

		const T& operator[](size_t index) const {
			ASSERT(allocation.block);
			ASSERT(n > index);

			return staticArray[index];
		}

		T& operator[](size_t index) {
			ASSERT(allocation.block);
			ASSERT(n > index);

			return staticArray[index];
		}
		T staticArray[n];
		Allocation allocation{ .block = staticArray,
							   .size = { n * sizeof(T) } };
	};

	template<typename T>
	struct BoundedArray<T, 0> {
		using ElementType = T;

		const T& operator[](size_t index) const {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity<T>(allocation) >= count);
			ASSERT(count > index);

			return ((const T*)allocation.block)[index];
		}

		T& operator[](size_t index) {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity<T>(allocation) >= count);
			ASSERT(count > index);

			return ((T*)allocation.block)[index];
		}

		Allocation allocation;
		size_t count;
	};

	template<typename T, size_t n>
	struct BoundedArray {
		using ElementType = T;

		const T& operator[](size_t index) const {
			ASSERT(allocation.block);
			ASSERT(n >= count);
			ASSERT(count > index);

			return staticArray[index];
		}

		T& operator[](size_t index) {
			ASSERT(allocation.block);
			ASSERT(n >= count);
			ASSERT(count > index);

			return staticArray[index];
		}
		T staticArray[n];
		Allocation allocation{ .block = staticArray,
							   .size = { n * sizeof(T) } };
		size_t count;
	};

	template<typename T, size_t n>
	constexpr size_t getCapacity(const FixedArray<T, n>& array) {
		size_t res{ n };
		return res;
	}

	template<typename T, size_t n>
	constexpr size_t getCapacity(const BoundedArray<T, n>& array) {
		size_t res{ n };
		return res;
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
		T lastElement{ (*array)[lastIndex] };
		(*array)[index] = lastElement;
		array->count--;
	}

	template<typename T, size_t n>
	void pushBack(BoundedArray<T, n>* array, const T& val) {
		ASSERT(array->allocation.block);
		ASSERT(array->count < pstd::getCapacity(*array));

		(*array)[array->count] = val;
		array->count++;
	}
	template<typename T, size_t n>
	bool find(
		const FixedArray<T, n>& array, const T& val, size_t* outIndex = nullptr
	) {
		ASSERT(out);
		ASSERT(array.allocation.block);

		size_t capacity{ pstd::getCapacity(array) };
		for (size_t i{}; i < capacity; i++) {
			if (array[i] == val) {
				if (outIndex) {
					*outIndex = i;
				}
				return true;
			}
		}

		return false;
	}

	template<typename T, size_t n, typename Callable>
	bool find(
		const BoundedArray<T, n>& array,
		Callable matchFunction,
		size_t* outIndex = nullptr
	) {
		ASSERT(array.allocation.block);
		ASSERT(array.count <= pstd::getCapacity(array));

		for (size_t i{}; i < array.count; i++) {
			if (matchFunction(array[i])) {
				if (outIndex) {
					*outIndex = i;
				}

				return true;
			}
		}

		return false;
	}

}  // namespace pstd
