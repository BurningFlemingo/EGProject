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

	struct FirstSetBit {
		uint32_t shift;
		bool found;
	};

	inline FirstSetBit bitscanForward(uint64_t val) {
		FirstSetBit result{};

		constexpr uint32_t nBitsInUInt{ 32 };
		for (int i{}; i < nBitsInUInt; i++) {
			if (val & (1 << i)) {
				result.found = true;
				result.shift = i;
				break;
			}
		}
		return result;
	}

};	// namespace pstd
