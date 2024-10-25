#include "public/PMath.h"

float pstd::sinf(float radians) {
	float x{ radians };
	float x3{ x * x * x };
	float x5{ x3 * x * x };

	float secondTerm{ x3 / 6.f };
	float thirdTerm{ x5 / 120.f };
	float res{ radians + secondTerm + thirdTerm };
	return res;
}

float pstd::sqrtf(float num) {
	float guess{ num / 2.f };
	const int nItterations{ 10 };
	for (int i{}; i < nItterations; i++) {
		guess = (guess + (num / guess)) * 0.5f;
	}
	return guess;
}

float pstd::powf(float num, uint32_t power) {
	float res{ 1 };
	for (int i{}; i < power; i++) {
		res *= num;
	}
	return res;
}

float pstd::fmodf(float num, uint32_t mod) {
	float res{};
	int32_t truncFloat{ (int32_t)num };
	if (num < 0) {
		truncFloat--;
	}
	float decimal{ num - truncFloat };
	res = (truncFloat % (int32_t)mod) + decimal;
	return res;
}

uint32_t pstd::factorial(uint32_t num) {
	uint32_t res{ 1 };
	while (num != 0) {
		res *= num;
		num--;
	}
	return num;
}

float pstd::absf(float num) {
	float res{ num };
	if (num < 0) {
		res *= -1;
	}
	return res;
}
