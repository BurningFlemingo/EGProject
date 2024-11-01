#pragma once
#include "PAssert.h"
#include "PArena.h"

namespace pstd {
	template<typename Container>
	typename Container::ElementType* getData(const Container& container) {
		return (typename Container::ElementType*)container.allocation.block;
	}

	template<typename Container>
	size_t getCapacity(const Container& container) {
		size_t res{ container.allocation.size *
					sizeof(Container::ElementType) };
		return res;
	}

}  // namespace pstd
