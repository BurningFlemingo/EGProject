#include "public/PMath.h"

using namespace pstd;

namespace Soft {
	float sqrtfNewton(const float num);
	float sinfTaylor(float radians);
	float atanfPade(const float ratio);
	float asinfPade(const float ratio);

}  // namespace Soft

float pstd::sqrtf(float num) {
	return Soft::sqrtfNewton(num);
}

float pstd::sinf(float radians) {
	return Soft::sinfTaylor(radians);
}

float pstd::atanf(const float ratio) {
	return Soft::atanfPade(ratio);
}

float pstd::atanf2(const float sin, const float cos) {
	float res{ atanf(sin / cos) };

	if (cos < 0) {
		if (sin < 0) {
			res -= PI;
		} else {
			res += PI;
		}
	}
	return res;
}

float pstd::asinf(const float ratio) {
	float res{};

	// domain restriction
	constexpr float epsilon{ 0.00001f };
	ASSERT(ratio < 1.f + epsilon);
	ASSERT(ratio > -1.f - epsilon);

	if (absf(ratio - 1.f) < epsilon) {
		res = HALF_PI;
		return res;
	}
	if (absf(ratio + 1.f) < epsilon) {
		res = -HALF_PI;
		return res;
	}

	if (absf(ratio) < epsilon) {
		res = 0;
		return res;
	}
	res = atanf(ratio / sqrtf(1.f - (ratio * ratio)));
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
		float res{};

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

	float atanfPade(const float ratio) {
		float res{};

		float x{ (float)ratio };

		constexpr float epsilon{ 0.00001f };
		if (absf(x) <= epsilon) {
			res = 0;
			return res;
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

		res = (numerator / denominator) + rangeAdjustment;

		return res;
	}

}  // namespace Soft
