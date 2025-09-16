#pragma once

#include "PAssert.h"
#include "PMemory.h"
#include "PTypes.h"
#include "PContainer.h"

namespace pstd {
	template<typename T, typename I = size_t>
	struct Array {	// <container type, index type>
		using ElementType = T;

		const T& operator[](I index) const {
			ASSERT(data);
			ASSERT(count > cast<size_t>(index));

			return data[cast<size_t>(index)];
		}

		T& operator[](I index) {
			ASSERT(data);
			ASSERT(count > cast<size_t>(index));

			return data[cast<size_t>(index)];
		}

		T* data;
		size_t capacity;
		size_t count{ capacity };
	};

	template<typename T, typename I = size_t>
	struct DArray {	 // <container type, index type>
		using ElementType = T;

		const T& operator[](I index) const {
			ASSERT(data);
			ASSERT(count > cast<size_t>(index));

			return data[cast<size_t>(index)];
		}

		T& operator[](I index) {
			ASSERT(data);
			ASSERT(count > cast<size_t>(index));

			return data[cast<size_t>(index)];
		}

		T* data;
		size_t count;
		size_t capacity;
		size_t commitSize;	// in bytes
	};

	template<typename T, size_t n, typename I = size_t>
	struct StaticArray {  // <container type, element count, index type>
		using ElementType = T;

		const T& operator[](I index) const {
			ASSERT(n > cast<size_t>(index));

			return data[cast<size_t>(index)];
		}

		T& operator[](I index) {
			ASSERT(n > cast<size_t>(index));

			return data[cast<size_t>(index)];
		}
		T data[n];
		size_t capacity{ n };
		size_t count{ n };
	};

	template<typename T, typename I = size_t>
	Array<T, I> createArray(Arena* pArena, size_t capacity, size_t count) {
		ASSERT(pArena);

		T* block{ pstd::alloc<T>(pArena, capacity) };

		return Array<T, I>{ .data = block,
							.capacity = capacity,
							.count = count };
	}

	template<typename T, typename I = size_t>
	Array<T, I> createArray(Arena* pArena, size_t capacity) {
		return createArray<T, I>(pArena, capacity, capacity);
	}

	template<typename T, typename I = size_t>
	DArray<T, I> createDArray(
		pstd::AllocationRegistry* pAllocRegistry,
		size_t initialCount = 0,
		size_t capacity = (4ll * GIB) / sizeof(T)
	) {
		ASSERT(initialCount <= capacity);

		size_t alignment{ alignof(T) };

		size_t capacitySize{ sizeof(T) * capacity };

		size_t countSize{ initialCount * sizeof(T) };
		size_t alignedCountSize{ (countSize + alignment - 1) &
								 ~(alignment - 1) };

		size_t commitSize{ min(capacitySize, countSize) };

		void* block{ heapAlloc(
			pAllocRegistry, capacitySize, alignment, pstd::ALLOC_RESERVED
		) };

		heapCommit(block, commitSize);

		return DArray<T, I>{ .data = rcast<T*>(block),
							 .count = initialCount,
							 .capacity = capacity,
							 .commitSize = commitSize };
	}

	template<typename T, typename I>
	void fill(Array<T, I>* pArray, T val) {
		for (size_t i{}; i < pArray->count; i++) {
			(*pArray)[ncast<I>(i)] = val;
		}
	}

	template<typename T, typename I>
	Array<T, I> makeConcatted(
		Arena* pArena, Array<T, I>* pLeftArray, Array<T, I>* pRightArray
	) {
		size_t newArrayCount{ pLeftArray->count + pRightArray->count };
		auto newArray{ createArray<T, I>(pArena, newArrayCount) };

		for (int i{}; i < pLeftArray->count; i++) {
			newArray[i] = pLeftArray[i];
		}
		for (int i{}; i < pRightArray->count; i++) {
			newArray[pLeftArray->count + i] = pRightArray[i];
		}

		return newArray;
	}

	template<typename T, typename I>
	void compactRemove(Array<T, I>* pArray, I index) {
		ASSERT(pArray);
		ASSERT(pArray->data);
		ASSERT(pArray->count <= pArray->capacity);
		ASSERT(index < pArray->count);

		if (pArray->count <= 1) {
			pArray->count = 0;
			return;
		}

		I lastIndex{ cast<I>(pArray->count - 1) };
		T lastElement{ (*pArray)[lastIndex] };
		(*pArray)[index] = lastElement;
		pArray->count--;
	}

	template<typename T, uint32_t n, typename I>
	void compactRemove(StaticArray<T, n, I>* pArray, I index) {
		ASSERT(pArray);
		ASSERT(pArray->count <= n);
		ASSERT(index < pArray->count);

		if (pArray->count <= 1) {
			pArray->count = 0;
			return;
		}

		I lastIndex{ cast<I>(pArray->count - 1) };
		T lastElement{ (*pArray)[lastIndex] };
		(*pArray)[index] = lastElement;
		pArray->count--;
	}

	template<typename T, typename I = size_t>
	void pushBack(DArray<T, I>* pArray, const T& val) {
		ASSERT(pArray->data);
		ASSERT(pArray->count < pArray->capacity);

		pArray->count++;
		size_t alignment{ alignof(T) };

		size_t capacitySize{ pArray->capacity * sizeof(T) };
		size_t countSize{ pArray->count * sizeof(T) };

		size_t growSize{};
		if (countSize > pArray->commitSize) {
			// if we need to grow and the full capacity has already been
			// fully committed, something has gone wrong, so
			// capacitySize should not equal pArray->commitSize here
			ASSERT(capacitySize > pArray->commitSize);

			growSize =
				min(capacitySize - pArray->commitSize, pArray->commitSize);
		}

		if (growSize > 0) {
			void* pCommitHead{ pArray->data + pArray->commitSize };

			heapCommit(pCommitHead, growSize);
			pArray->commitSize += growSize;
		}

		auto index{ ncast<I>(pArray->count - 1) };
		(*pArray)[index] = val;
	}

	template<typename T, typename I>
	void pushBack(Array<T, I>* pArray, const T& val) {
		ASSERT(pArray->data);
		ASSERT(pArray->count < pArray->capacity);

		pArray->count++;
		auto index{ ncast<I>(pArray->count - 1) };
		(*pArray)[index] = val;
	}

	template<typename T, size_t n, typename I>
	void pushBack(StaticArray<T, n, I>* array, const T& val) {
		ASSERT(array);
		ASSERT(array->count < n);

		array->count++;
		auto index{ ncast<I>(array->count - 1) };
		(*array)[index] = val;
	}

}  // namespace pstd
