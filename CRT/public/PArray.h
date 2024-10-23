#pragma once
#include "PAssert.h"
#include <stdint.h>

namespace pstd {
	template<typename T>
	struct FixedArray {
		T& operator[](const size_t index) {
			ASSERT(index < capacity);

			T& element{ arr[index] };
			return element;
		}

		void pushBack(T element) {
			ASSERT(occupancy < capacity);
			arr[occupancy] = element;
			occupancy++;
		}

		T popBack() {
			ASSERT(occupancy > 0);
			occupancy--;
			T element{ arr[occupancy] };
			return element;
		}

		T* arr;
		size_t capacity;
		size_t occupancy;
	};
}  // namespace pstd
