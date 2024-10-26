#pragma once

#include "PTypes.h"

namespace pstd {
	constexpr float PI{ 3.14159265f };
	constexpr float TAU{ 2.f * PI };
	constexpr float HALF_PI{ PI / 2.f };

	template<typename T>
	constexpr float roundf(const T num) {
		float res{};
		if (num < 0) {
			res = (int)(num - 0.5f);
		} else {
			res = (int)(num + 0.5f);
		}
		return res;
	}

	template<typename T>
	constexpr float powf(const T num, const uint32_t power) {
		float res{ 1 };
		for (int i{}; i < power; i++) {
			res *= num;
		}
		return res;
	}

	template<typename T>
	constexpr float fmodf(const T num, const uint32_t mod) {
		float res{};
		int32_t truncFloat{ (int32_t)num };
		if (num < 0) {
			truncFloat--;
		}
		float decimal{ num - truncFloat };
		res = (truncFloat % (int32_t)mod) + decimal;
		return res;
	}

	template<typename T>
	constexpr float absf(const T num) {
		float res{ num };
		if (num < 0) {
			res *= -1;
		}
		return res;
	}

	template<typename T>
	constexpr T getSign(const T num) {
		T res{ 1 };
		if (num < 0) {
			res = -1;
		}
		return res;
	}

	template<typename T>
	constexpr float sinfTaylor(const T radians) {
		float res{};

		// makes x in between -TAU and TAU
		float x{ fmodf(radians / PI, 2) };
		x *= PI;

		// makes x in between -pi and pi
		if (x > PI) {
			x -= TAU;
		}
		if (x < -PI) {
			x += TAU;
		}

		constexpr float epsilon{ 0.00001f };
		if (absf(x) <= epsilon) {
			res = 0.f;
			return res;
		}
		if (absf(x - PI) <= epsilon || absf(x + PI) <= epsilon) {
			res = 0.0f;
			return res;
		}
		if (absf(x - HALF_PI) <= epsilon) {
			res = 1.f;
			return res;
		}
		if (absf(x + HALF_PI) <= epsilon) {
			res = -1.f;
			return res;
		}

		// makes x in between -pi/2 and pi/2
		if (x > HALF_PI) {
			x = PI - x;
		}
		if (x < -HALF_PI) {
			x = -PI - x;
		}

		float x2{ x * x };
		float x3{ x * x2 };
		float x5{ x3 * x2 };
		float x7{ x5 * x2 };

		float firstTerm{ x };
		float secondTerm{ x3 / 6.f };
		float thirdTerm{ x5 / 120.f };
		float fourthTerm{ x7 / 5040.f };

		res = firstTerm - secondTerm + thirdTerm - fourthTerm;

		return res;
	}

	template<typename T>
	constexpr float cosfTaylor(const T radians) {
		float res{ sinfTaylor(radians + 0.5f * PI) };
		return res;
	}

	template<typename T>
	constexpr float sqrtfNewton(const T num) {
		ASSERT(num >= 0);
		if (num == 0) {
			return 0;
		}

		float guess{ num / 2.f };
		float prevGuess{};
		constexpr int maxItterations{ 10 };
		constexpr float tolerance{ 0.00001f };
		float epsilon{ tolerance * num };
		for (int i{}; i < maxItterations; i++) {
			prevGuess = guess;
			guess = (guess + (num / guess)) * 0.5f;

			if (absf(guess - prevGuess) <= epsilon) {
				break;
			}
		}
		return guess;
	}
}  // namespace pstd
