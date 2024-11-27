#pragma once
#include "PAssert.h"
#include "PArena.h"

namespace pstd {

	template<typename T>
	concept Container = requires(T arg) { typename T::ElementType; };

	template<typename T>
	concept CountableContainer = Container<T> && requires(T arg) { arg.count; };

	template<typename T>
	concept StaticContainer = Container<T> && requires(T arg) {
		typename T::ElementType;

		arg.data[getCapacity(arg) - 1];	 // .data implies the container doesnt
										 // have an allocation, and the index is
										 // to make sure the size is correct
	};

	template<Container T>
	size_t getCapacity(const T& container) {
		size_t res{ container.allocation.size / sizeof(T::ElementType) };
		return res;
	}

	template<Container T>
	size_t getCount(const T& container) {
		return container.count;
	}

	template<Container T>
	size_t getAddressable(const T& container) {
		return pstd::getCapacity(container);
	}

	template<CountableContainer T>
	size_t getAddressable(const T& container) {
		return container.count;
	}

	template<Container T>
	typename T::ElementType* getData(const T& container) {
		return (typename T::ElementType*)container.allocation.block;
	}

	template<typename T>
		requires StaticContainer<T>
	constexpr typename T::ElementType* getData(const T& container) {
		return (typename T::ElementType*)container.data;
	}

	template<Container T>
	bool find(const T& container, const T& val, size_t* outIndex = nullptr) {
		size_t capacity{ pstd::getAddressable(container) };
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

	template<typename Container, typename Callable>
	bool find(
		const Container& container,
		Callable matchFunction,
		size_t* outIndex = nullptr
	) {
		for (size_t i{}; i < pstd::getAddressable(container); i++) {
			if (matchFunction(container[i])) {
				if (outIndex) {
					*outIndex = i;
				}

				return true;
			}
		}

		return false;
	}

}  // namespace pstd
