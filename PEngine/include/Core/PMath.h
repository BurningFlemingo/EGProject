#pragma once

#include "PTypes.h"
#include "PAssert.h"

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

	float sqrtf(float num);

	float sinf(float radians);

	inline float cosf(const float radians) {
		float res{ sinf(radians + 0.5 * PI) };
		return res;
	}

	float tanf(const float radians);

	float atanf(const float ratio);
	float atanf2(const float sin, const float cos);

	float asinf(const float ratio);

	inline float acosf(const float ratio) {
		return HALF_PI - asinf(ratio);
	}

}  // namespace pstd
