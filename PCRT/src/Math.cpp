#include "public/PMath.h"
#include <windows.h>

using namespace pstd;

namespace Soft {
	float sqrtfNewton(const float num);
	float sinfTaylor(float radians);
	float atanfPade(const float ratio);
	float asinfPade(const float ratio);

}  // namespace Soft

float pstd::sqrtf(float num) {
	float res{ Soft::sqrtfNewton(num) };
	return res;
}

float pstd::sinf(float radians) {
	float res{ Soft::sinfTaylor(radians) };
	return res;
}

float pstd::tanf(const float radians) {
	float res{};

	float cos{ cosf(radians) };
	if (cos == 0) {
		return res;
	}
	res = sinf(radians) / cosf(radians);
	return res;
}

float pstd::atanf(const float ratio) {
	float res{ Soft::atanfPade(ratio) };
	return res;
}

float pstd::atanf2(const float y, const float x) {
	if (x == 0) {
		if (y < 0) {
			return -HALF_PI;
		} else if (y > 0) {
			return HALF_PI;
		}
		return 0;
	}

	float res{};
	float atan{ atanf(y / x) };
	if (x < 0) {
		if (y < 0) {
			res = atan - PI;
		} else {
			res = atan + PI;
		}
		return res;
	}

	res = atan;
	return res;
}

float pstd::asinf(const float ratio) {
	// domain restriction
	constexpr float epsilon{ 0.00001f };
	ASSERT(ratio < 1.f + epsilon);
	ASSERT(ratio > -1.f - epsilon);

	if (absf(ratio - 1.f) < epsilon) {
		return HALF_PI;
	}
	if (absf(ratio + 1.f) < epsilon) {
		return -HALF_PI;
	}

	if (absf(ratio) < epsilon) {
		return 0;
	}
	float res{ atanf(ratio / sqrtf(1.f - (ratio * ratio))) };
	return res;
}

namespace Soft {
	float sqrtfNewton(const float num) {
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

	float sinfTaylor(float radians) {
		// makes x in between -TAU and TAU
		float x{ fmodf((float)radians / PI, 2) };
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
			return 0.f;
		}
		if (absf(x - PI) <= epsilon || absf(x + PI) <= epsilon) {
			return 0.f;
		}
		if (absf(x - HALF_PI) <= epsilon) {
			return 1.f;
		}
		if (absf(x + HALF_PI) <= epsilon) {
			return -1.f;
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

		return firstTerm - secondTerm + thirdTerm - fourthTerm;
	}

	float atanfPade(const float ratio) {
		float x{ (float)ratio };

		constexpr float epsilon{ 0.00001f };
		if (absf(x) <= epsilon) {
			return 0;
		}

		// for |x| > 1, the approximation becomes extremely innacurate, so
		// the identity atan(x) = -atan(x) +- pi/2 is used instead
		float rangeAdjustment{};
		if (x >= 1.f) {
			rangeAdjustment = HALF_PI;
			x = -1.f / x;
		} else if (x < -1.f) {
			rangeAdjustment = -HALF_PI;
			x = -1.f / x;
		}

		float x2{ x * x };
		float numerator{ 15.f * x + 4.f * x2 * x };
		float denominator{ 15.f + 9.f * x2 };

		return (numerator / denominator) + rangeAdjustment;
	}

}  // namespace Soft
