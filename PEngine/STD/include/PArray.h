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
		size_t count;
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
		size_t count{ n };
	};

	template<typename T, typename I = size_t>
	struct BoundedArray {  // <element type, index type>
		using ElementType = T;

		const T& operator[](I index) const {
			ASSERT(data);
			ASSERT(capacity >= count);
			ASSERT(count > index);

			return rcast<const T*>(data)[cast<size_t>(index)];
		}

		T& operator[](I index) {
			ASSERT(data);
			ASSERT(capacity >= count);
			ASSERT(count > index);

			return rcast<T*>(data)[cast<size_t>(index)];
		}

		T* data;
		size_t capacity;
		size_t count{};
	};

	template<typename T, size_t n, typename I = size_t>
	struct BoundedStaticArray {	 // <container type, element count, index
								 // type>
		using ElementType = T;

		const T& operator[](I index) const {
			ASSERT(n > index);
			ASSERT(n >= count);
			ASSERT(count > index);

			return data[cast<size_t>(index)];
		}

		T& operator[](I index) {
			ASSERT(n > index);
			ASSERT(count > index);

			return data[cast<size_t>(index)];
		}
		T data[n];
		size_t count;
	};

	template<typename T, typename I = size_t>
	Array<T, I> createArray(Arena* pArena, size_t count) {
		ASSERT(pArena);

		Allocation allocation{ pstd::alloc<T>(pArena, count) };

		return Array<T, I>{ .data = rcast<T*>(allocation.block),
							.count = count };
	}

	template<typename T, typename I = size_t>
	Array<T, I> createArray(const Allocation& allocation) {
		ASSERT(allocation.block);

		size_t elementSize{ sizeof(T) };
		size_t count{ allocation.size / elementSize };

		return Array<T, I>{ .data = rcast<T*>(allocation.block),
							.count = count };
	}

	template<typename T, typename I = size_t>
	BoundedArray<T, I> createBoundedArray(
		Arena* pArena, size_t capacity, size_t startCount = 0
	) {
		ASSERT(pArena);

		Allocation allocation{ pstd::alloc<T>(pArena, capacity) };

		ASSERT(capacity >= startCount);

		return BoundedArray<T, I>{ .data = rcast<T*>(allocation.block),
								   .capacity = capacity,
								   .count = startCount };
	}

	template<typename T, typename I = size_t>
	BoundedArray<T, I> createBoundedArray(const Allocation& allocation) {
		ASSERT(allocation.block);

		size_t count{ allocation.size / sizeof(T) };

		return BoundedArray<T, I>{ .data = rcast<T*>(allocation.block),
								   .capacity = count,
								   .count = count };
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

		Allocation reservedAlloc{ heapAlloc(
			pAllocRegistry, capacitySize, alignment, pstd::ALLOC_RESERVED
		) };

		heapCommit(reservedAlloc.block, commitSize);

		return DArray<T, I>{ .data = rcast<T*>(reservedAlloc.block),
							 .count = initialCount,
							 .capacity = capacity,
							 .commitSize = commitSize };
	}

	template<typename T, typename I = size_t>
	Allocation getAllocation(const Array<T, I>& array) {
		ASSERT(array.data);
		size_t size{ array.count * sizeof(T) };
		return Allocation{ .block = rcast<uint8_t*>(array.data), .size = size };
	}

	template<typename T, typename I>
	void fill(Array<T, I>* pArray, T val) {
		for (size_t i{}; i < pArray->count; i++) {
			(*pArray)[ncast<I>(i)] = val;
		}
	}

	template<typename T, typename I = size_t>
	void fill(BoundedArray<T, I>* pArray, T val) {
		for (size_t i{}; i < pArray->count; i++) {
			(*pArray)[ncast<I>(i)] = val;
		}
	}

	template<typename T, typename I>
	void compactRemove(BoundedArray<T, I>* pArray, I index) {
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
	void compactRemove(BoundedStaticArray<T, n, I>* pArray, I index) {
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
	void pushBack(BoundedArray<T, I>* pArray, const T& val) {
		ASSERT(pArray->data);
		ASSERT(pArray->count < pArray->capacity);

		pArray->count++;
		auto index{ ncast<I>(pArray->count - 1) };
		(*pArray)[index] = val;
	}
	template<typename T, size_t n, typename I>
	void pushBack(BoundedStaticArray<T, n, I>* array, const T& val) {
		ASSERT(array);
		ASSERT(array->count < n);

		array->count++;
		auto index{ ncast<I>(array->count - 1) };
		(*array)[index] = val;
	}

}  // namespace pstd
