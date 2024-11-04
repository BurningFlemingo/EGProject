#pragma once
#include "PAssert.h"
#include "PMemory.h"
#include "PTypes.h"
#include "PArena.h"
#include "PContainer.h"

namespace pstd {
	template<typename T>
	struct FixedArray {
		using ElementType = T;
		using IsContainerType = T;

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

	template<typename T, uint32_t n>
	struct StackArray {
		using ElementType = T;

		const T& operator[](size_t index) const {
			ASSERT(allocation.block);
			ASSERT(n > index);

			return data[index];
		}

		T& operator[](size_t index) {
			ASSERT(allocation.block);
			ASSERT(n > index);

			return data[index];
		}
		T data[n];
	};

	template<typename T>
	struct BoundedArray {
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

	template<typename T, uint32_t n>
	struct BoundedStackArray {
		using ElementType = T;

		const T& operator[](size_t index) const {
			ASSERT(allocation.block);
			ASSERT(n > index);
			ASSERT(n >= count);
			ASSERT(count > index);

			return data[index];
		}

		T& operator[](size_t index) {
			ASSERT(allocation.block);
			ASSERT(n > index);
			ASSERT(count > index);

			return data[index];
		}
		T data[n];
		size_t count;
	};

	template<Container T, uint32_t n>
	size_t getCapacity(const StackArray<T, n>& container) {
		size_t res{ n };
		return res;
	}
	template<Container T, uint32_t n>
	size_t getCapacity(const BoundedStackArray<T, n>& container) {
		size_t res{ n };
		return res;
	}

	template<typename T, uint32_t n>
	T* getData(const StackArray<T, n>& container) {
		return container.data;
	}

	template<typename T, uint32_t n>
	T* getData(const BoundedStackArray<T, n>& container) {
		return container.data;
	}

	template<typename T, uint32_t n>
	constexpr const Allocation
		getStackAllocation(const StackArray<T, n> container) {
		Allocation allocation{ .block = (void*)container.data,
							   .size = n * sizeof(T),
							   .isStackAllocated = true };
		return allocation;
	}
	template<typename T, uint32_t n>
	constexpr Allocation
		getStackAllocation(const BoundedStackArray<T, n> container) {
		Allocation allocation{ .block = (void*)container.data,
							   .size = n * sizeof(T),
							   .isStackAllocated = true };
		return allocation;
	}

	template<typename T>
	void compactRemove(BoundedArray<T>* array, size_t index) {
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
	template<typename T, uint32_t n>
	void compactRemove(BoundedStackArray<T, n>* array, size_t index) {
		ASSERT(array);
		ASSERT(array->allocation.block);
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

	template<typename T>
	void pushBack(BoundedArray<T>* array, const T& val) {
		ASSERT(array->allocation.block);
		ASSERT(array->count < pstd::getCapacity(*array));

		(*array)[array->count] = val;
		array->count++;
	}
	template<typename T, size_t n>
	void pushBack(BoundedStackArray<T, n>* array, const T& val) {
		ASSERT(array);
		ASSERT(array->allocation.block);
		ASSERT(array->count < n);

		(*array)[array->count] = val;
		array->count++;
	}

}  // namespace pstd
