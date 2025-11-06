#pragma once

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)

namespace pstd {
	template<typename T>
	T clamp(T min, T max, T val) {
		if (val <= min) {
			return min;
		}
		if (val >= max) {
			return max;
		}
		return val;
	}
};	// namespace pstd
