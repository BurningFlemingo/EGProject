#pragma once
#include "PAssert.h"
#include "PMemory.h"
#include "PTypes.h"
#include "PArena.h"
#include "PContainer.h"

namespace pstd {
	template<typename T, typename I = size_t>
	struct Array {	// <container type, index type>
		using ElementType = T;

		const T& operator[](I index) const {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity<T>(allocation) > index);

			return rcast<const T*>(allocation.block)[cast<size_t>(index)];
		}

		T& operator[](I index) {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity<T>(allocation) > index);

			return rcast<T*>(allocation.block)[cast<size_t>(index)];
		}
		Allocation allocation;
	};

	template<typename T, uint32_t n, typename I = size_t>
	struct StaticArray {  // <container type, element count, index type>
		using ElementType = T;

		const T& operator[](I index) const {
			ASSERT(n > index);

			return data[cast<size_t>(index)];
		}

		T& operator[](I index) {
			ASSERT(n > index);

			return data[cast<size_t>(index)];
		}
		T data[n];
	};

	template<typename T, typename I = size_t>
	struct BoundedArray {  // <element type, index type>
		using ElementType = T;

		const T& operator[](I index) const {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity<T>(allocation) >= count);
			ASSERT(count > index);

			return rcast<const T*>(allocation.block)[cast<size_t>(index)];
		}

		T& operator[](I index) {
			ASSERT(allocation.block);
			ASSERT(pstd::getCapacity<T>(allocation) >= count);
			ASSERT(count > index);

			return rcast<T*>(allocation.block)[cast<size_t>(index)];
		}

		Allocation allocation;
		size_t count;
	};

	template<typename T, uint32_t n, typename I = size_t>
	struct BoundedStaticArray {	 // <container type, element count, index type>
		using ElementType = T;

		const T& operator[](I index) const {
			ASSERT(allocation.block);
			ASSERT(n > index);
			ASSERT(n >= count);
			ASSERT(count > index);

			return data[cast<size_t>(index)];
		}

		T& operator[](I index) {
			ASSERT(allocation.block);
			ASSERT(n > index);
			ASSERT(count > index);

			return data[cast<size_t>(index)];
		}
		T data[n];
		size_t count;
	};

	template<typename T, uint32_t n>
	constexpr size_t getCapacity(const StaticArray<T, n>& array) {
		size_t res{ n };
		return res;
	}
	template<typename T, uint32_t n, typename I>
	constexpr size_t getCapacity(const BoundedStaticArray<T, n, I>& array) {
		size_t res{ n };
		return res;
	}

	template<typename T, typename I>
	void fill(Array<T, I>* arena, T val) {
		for (size_t i{}; i < pstd::getCapacity(*arena); i++) {
			(*arena)[(I)i] = val;
		}
	}

	template<typename T, typename I>
	void compactRemove(BoundedArray<T, I>* array, size_t index) {
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
	template<typename T, uint32_t n, typename I>
	void compactRemove(BoundedStaticArray<T, n, I>* array, size_t index) {
		ASSERT(array);
		ASSERT(array->count <= n);
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

	template<typename T, typename I>
	void pushBack(BoundedArray<T, I>* array, const T& val) {
		ASSERT(array->allocation.block);
		ASSERT(array->count < pstd::getCapacity(*array));

		(*array)[array->count] = val;
		array->count++;
	}
	template<typename T, size_t n, typename I>
	void pushBack(BoundedStaticArray<T, n, I>* array, const T& val) {
		ASSERT(array);
		ASSERT(array->count < n);

		(*array)[array->count] = val;
		array->count++;
	}

}  // namespace pstd
