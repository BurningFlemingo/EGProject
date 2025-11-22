#include "STD/PAlgorithm.h"

template<typename T>
pstd::FirstSetBit pstd::bitscanForward(const T val) {
	FirstSetBit result{};

	constexpr uint32_t valBitSize{ sizeof(val) * 8 };
	for (uint32_t i{}; i < valBitSize; i++) {
		if (val & (ncast<T>(1) << i)) {
			result.found = true;
			result.shift = i;
			break;
		}
	}
	return result;
}

template pstd::FirstSetBit pstd::bitscanForward(uint32_t val);
template pstd::FirstSetBit pstd::bitscanForward(uint64_t val);
