#pragma once
#include <stdint.h>
#include "PAssert.h"

namespace pstd {
	template<uint32_t count, typename T>
	struct v;

	template<typename T>
	struct v<2, T> {
		union {
			T elements[2];
			struct {
				T x;
				T y;
			};
		};
	};

	template<typename T>
	struct v<3, T> {
		union {
			T elements[3];
			struct {
				T x;
				T y;
				T z;
			};
		};
	};

	template<typename T>
	struct v<4, T> {
		union {
			T elements[4];
			struct {
				T x;
				T y;
				T z;
				T w;
			};
		};
	};
	template<typename T>
	using v2 = v<2, T>;

	template<typename T>
	using v3 = v<2, T>;

	template<typename T>
	using v4 = v<4, T>;

	template<uint32_t n, typename T>
	v<n, T> operator+(const v<n, T>& a, const v<n, T>& b) {
		ASSERT(n >= 2 && n <= 4);

		v<n, T> res{};
		for (uint32_t i{}; i < n; i++) {
			res.elements[i] = a.elements[i] + b.elements[i];
		}
		return res;
	}

	template<uint32_t n, typename T>
	v<n, T> operator-(const v<n, T>& a, const v<n, T>& b) {
		ASSERT(n >= 2 && n <= 4);

		v<n, T> res{};
		for (uint32_t i{}; i < n; i++) {
			res.elements[i] = a.elements[i] - b.elements[i];
		}
		return res;
	}

	template<uint32_t n, typename T>
	v<n, T> operator/(const v<n, T>& a, const v<n, T>& b) {
		ASSERT(n >= 2 && n <= 4);

		v<n, T> res{};
		for (uint32_t i{}; i < n; i++) {
			res.elements[i] = a.elements[i] / b.elements[i];
		}
		return res;
	}

	template<uint32_t n, typename T>
	v<n, T> operator*(const v<n, T>& a, const v<n, T>& b) {
		ASSERT(n >= 2 && n <= 4);

		v<n, T> res{};
		for (uint32_t i{}; i < n; i++) {
			res.elements[i] = a.elements[i] * b.elements[i];
		}
		return res;
	}
	template<uint32_t n, typename T>
	v<n, T> dot(const v<n, T>& a, const v<n, T>& b) {
		ASSERT(n >= 2 && n <= 4);

		T res{};
		for (uint32_t i{}; i < n; i++) {
			res += a.elements[i] * b.elements[i];
		}
		return res;
	}

}  // namespace pstd
