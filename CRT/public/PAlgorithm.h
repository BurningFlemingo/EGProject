#pragma once
#include <stdint.h>

namespace pstd {
	template<typename T>
	T min(T a, T b) {
		T res{ a };
		if (a > b) {
			res = b;
		}
		return res;
	}
};	// namespace pstd
