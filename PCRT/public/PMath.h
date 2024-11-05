#pragma once

#include "PTypes.h"
#include "PAssert.h"
#include "PSTDAPI.h"

namespace pstd {
	constexpr float PI{ 3.14159265f };
	constexpr float TAU{ 2.f * PI };
	constexpr float HALF_PI{ PI / 2.f };

	template<typename T>
	T pow(const T num, const uint32_t power) {
		T res{ 1 };
		for (int i{}; i < power; i++) {
			res *= num;
		}
		return res;
	}

	template<typename T>
	T abs(const T num) {
		T res{ num };
		if (num < 0) {
			return -res;
		}

		return res;
	}

	template<typename T>
	constexpr float roundf(const T num) {
		float res{ (int)(num + 0.5f) };
		if (num < 0) {
			res = (int)(num - 0.5f);
			return res;
		}
		return res;
	}

	template<typename T>
	constexpr float fmodf(const T num, const uint32_t mod) {
		int32_t truncFloat{ (int32_t)num };
		if (num < 0) {
			truncFloat--;
		}
		float decimal{ num - truncFloat };
		float res{ (truncFloat % (int32_t)mod) + decimal };
		return res;
	}

	template<typename T>
	constexpr float absf(const T num) {
		float res{ num };
		if (num < 0) {
			return -res;
		}

		return res;
	}

	template<typename T>
	constexpr T getSign(const T num) {
		T res{ 1 };
		if (num < 0) {
			res = -1;
			return res;
		}
		return res;
	}

	PSTD_API float sqrtf(float num);

	PSTD_API float sinf(float radians);

	inline float cosf(const float radians) {
		float res{ sinf(radians + 0.5 * PI) };
		return res;
	}

	PSTD_API float tanf(const float radians);

	PSTD_API float atanf(const float ratio);
	PSTD_API float atanf2(const float sin, const float cos);

	PSTD_API float asinf(const float ratio);

	inline float acosf(const float ratio) {
		return HALF_PI - asinf(ratio);
	}

}  // namespace pstd
