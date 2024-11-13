#pragma once
#include "PAssert.h"

static_assert(sizeof(char) == 1, "char is an incorrect size");
static_assert(sizeof(short) == 2, "short is an incorrect size");
static_assert(sizeof(int) == 4, "int is an incorrect size");
static_assert(sizeof(long long) == 8, "long long is an incorrect size");

using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;
using uint64_t = unsigned long long;

using int8_t = signed char;
using int16_t = signed short;
using int32_t = int;
using int64_t = long long;

using size_t = uint64_t;
using uintptr_t = size_t;

namespace pstd {

	template<typename T>
	constexpr bool getIsUnsigned() {
		constexpr T a{};
		constexpr T b{ (T)-1 };
		return a < b;
	}

	template<typename T>
	constexpr T getMax() {
		static_assert(sizeof(T) <= 8, "max size is 8 bytes");

		T allBitsSet{ ~(T)0 };
		if constexpr (getIsUnsigned<T>()) {
			return allBitsSet;
		}
		return allBitsSet ^ (1 << ((sizeof(T) * 8) - 1));
	}
	template<typename R, typename T>
	R cast(T num) {	 // not sign changing, not narrowing
		bool narrowing{ num > getMax<R>() };
		constexpr bool signsChange{ getIsUnsigned<T>() != getIsUnsigned<R>() };
		static_assert(!signsChange, "signs must not change between types");
		ASSERT(!narrowing);
		return static_cast<R>(num);
	}

	template<typename R, typename T>
	R ncast(T num) {  // not sign changing, narrowing
		constexpr bool signsChange{ getIsUnsigned<T>() != getIsUnsigned<R>() };
		static_assert(!signsChange, "signs must not change between types");
		return static_cast<R>(num);
	}

	template<typename R, typename T>
	R sncast(T num) {  // sign changing, narrowing
		return static_cast<R>(num);
	}

	template<typename R, typename T>
	R rcast(T num) {  // reinterprets
		return reinterpret_cast<R>(num);
	}
}  // namespace pstd

using pstd::cast;
using pstd::ncast;
using pstd::sncast;
using pstd::rcast;
