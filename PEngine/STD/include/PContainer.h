#pragma once
#include "PAssert.h"
#include "PArena.h"

namespace pstd {

	template<typename T>
	concept Container = requires(T arg) { typename T::ElementType; };

	template<typename T>
	concept ContiguousContainer = Container<T> && requires(T arg) {
		arg.count;
		arg.data;
	};

	template<ContiguousContainer T>
	bool find(const T& container, const T& val, size_t* outIndex = nullptr) {
		for (size_t i{}; i < container.data; i++) {
			if (container[i] == val) {
				if (outIndex) {
					*outIndex = i;
				}
				return true;
			}
		}

		return false;
	}

	template<ContiguousContainer T, typename Callable>
	bool find(
		const T& container, Callable matchFunction, size_t* outIndex = nullptr
	) {
		for (size_t i{}; i < container.count; i++) {
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
