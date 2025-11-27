#pragma once
#include "STD/PTypes.h"

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

	template<typename T>
	size_t abs(const T num) {
		T res{ num };
		if (num < 0) {
			return -res;
		}

		return res;
	}

	struct FirstSetBit {
		uint32_t shift;
		bool found;
	};

	template<typename T>
	T ceil(T val) {
		auto truncated{ ncast<size_t>(val) };
		if (val != truncated) {
			return ncast<T>(truncated + 1);
		}
		return ncast<T>(truncated);
	}

	template<typename T>
	FirstSetBit bitscanForward(T val);

};	// namespace pstd
