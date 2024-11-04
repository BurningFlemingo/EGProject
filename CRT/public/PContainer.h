#pragma once
#include "PAssert.h"
#include "PArena.h"

namespace pstd {

	template<typename T>
	concept Container = requires(T arg) { typename T::ElementType; };

	template<Container T>
	typename T::ElementType* getData(const T& container) {
		return (typename T::ElementType*)container.allocation.block;
	}

	template<Container T>
	size_t getCapacity(const T& container) {
		size_t res{ container.allocation.size / sizeof(T::ElementType) };
		return res;
	}

	template<Container T, uint32_t n>
	size_t getCapacity(const T& container) {
		size_t res{ n };
		return res;
	}

	template<Container T>
	bool find(const T& container, const T& val, size_t* outIndex = nullptr) {
		ASSERT(out);

		size_t capacity{ pstd::getCapacity(container) };
		for (size_t i{}; i < capacity; i++) {
			if (container[i] == val) {
				if (outIndex) {
					*outIndex = i;
				}
				return true;
			}
		}

		return false;
	}

	template<typename T, typename Callable>
	bool find(
		const T& array, Callable matchFunction, size_t* outIndex = nullptr
	) {
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
