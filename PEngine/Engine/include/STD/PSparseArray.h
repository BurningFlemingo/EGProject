#pragma once

#include "PHashMap.h"
#include "PArray.h"

namespace pstd {
	template<typename K, typename T>
	struct SparseArray {
		T& operator[](K key) {
			size_t index{};
			if (!pstd::find(&keyToIndex, key, &index)) {
				ASSERT(denseArray.count < denseArray.capacity);

				keyToIndex[key] = denseArray.count;
				index = denseArray.count;
				denseArray.count++;
			}

			return denseArray[index];
		}

		const T& operator[](K key) const {
			size_t index{ keyToIndex[key] };
			return denseArray[index];
		}

		pstd::HashMap<K, size_t> keyToIndex;
		pstd::Array<T> denseArray;
	};

	template<typename K, typename T>
	SparseArray<K, T> createSparseArray(pstd::Arena* pArena, size_t capacity) {
		return SparseArray<K, T>{
			.keyToIndex = pstd::createHashMap<K, T>(pArena, capacity),
			.denseArray = pstd::createArray<T>(pArena, capacity, 0)
		};
	}

	template<typename K, typename T>
	bool contains(const SparseArray<K, T>& array, K key) {
		return pstd::contains(array.keyToIndex, key);
	}

}  // namespace pstd
