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
	concept DecimalType = requires { T{ 1.5 } || T{ 1.5f }; };

	template<typename R, typename T>
	bool getIsNarrowing(T num) {
		if (static_cast<R>(num) != num) {
			return true;
		}
		return false;
	}

	template<typename T>
	constexpr bool getIsUnsigned() {
		constexpr T a{};
		constexpr T b{ (T)-1 };
		return a < b;
	}

	template<typename R, typename T>
	constexpr bool getCanNarrow() {
		constexpr bool signsChange{ getIsUnsigned<R>() != getIsUnsigned<T>() };
		constexpr int losesBytes{ (sizeof(R) < sizeof(T)) };
		if constexpr (signsChange || losesBytes) {
			return true;
		}
		return false;
	}

	template<typename R, typename T>
		requires((DecimalType<R> ^ DecimalType<T>) > 0)
	constexpr bool getCanNarrow() {
		return true;
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
	constexpr R cast(T num) {  // types dont narrow
		static_assert(
			!getCanNarrow<R, T>(), "types should not narrow on this cast"
		);
		return static_cast<R>(num);
	}

	template<typename R, typename T>
	R vcast(T num) {  // value doesnt narrow
		ASSERT(static_cast<R>(num) == num);
		return static_cast<R>(num);
	}

	template<typename R, typename T>
	constexpr R ncast(T num) {	// narrowing cast
		return static_cast<R>(num);
	}

	template<typename R, typename T>
	constexpr R rcast(T num) {	// reinterpret cast
		return reinterpret_cast<R>(num);
	}

	template<typename T>
	constexpr T&& move(T num) {
		return static_cast<T&&>(num);
	}

}  // namespace pstd

using pstd::cast;
using pstd::vcast;
using pstd::ncast;
using pstd::rcast;
