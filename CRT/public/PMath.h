#pragma once
#include <stdint.h>
#include <type_traits>

namespace pstd {
	constexpr float PI{ 3.141592f };
	constexpr float TAU{ 2.f * 3.141592f };
	float sinf(float radians);
	float sqrtf(float num);
	float powf(float num, uint32_t power);
	float fmodf(float num, uint32_t mod);
	uint32_t factorial(uint32_t num);
	float absf(float num);

	template<typename T>
		requires std::is_arithmetic_v<T>
	float sin(T radians);
}  // namespace pstd
