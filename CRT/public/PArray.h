#pragma once
#include "PAssert.h"
#include <stdint.h>

namespace pstd {
	template<typename T, uint16_t count = 1>
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

		T arr[count];
		size_t capacity{ count };
		size_t occupancy;
	};
}  // namespace pstd
