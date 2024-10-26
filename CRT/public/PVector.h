#pragma once
#include "PTypes.h"
#include "PAssert.h"

namespace pstd {
	template<uint32_t count, typename T>
	struct v {
		v() = default;
	};

	template<typename T>
	struct v<2, T> {
		T& operator[](const size_t index) {
			ASSERT(index < 2);
			T& res{ e[index] };
			return res;
		}
		const T& operator[](const size_t index) const {
			ASSERT(index < 2);
			const T& res{ e[index] };
			return res;
		}
		union {
			T e[2];
			struct {
				T x;
				T y;
			};
		};
	};

	template<typename T>
	struct v<3, T> {
		T& operator[](const size_t index) {
			ASSERT(index < 3);
			T& res{ e[index] };
			return res;
		}
		const T& operator[](const size_t index) const {
			ASSERT(index < 3);
			const T& res{ e[index] };
			return res;
		}
		union {
			T e[3];
			struct {
				T x;
				T y;
				T z;
			};
		};
	};

	template<typename T>
	struct v<4, T> {
		T& operator[](const size_t index) {
			ASSERT(index < 4);
			T& res{ e[index] };
			return res;
		}
		const T& operator[](const size_t index) const {
			ASSERT(index < 4);
			const T& res{ e[index] };
			return res;
		}
		union {
			T e[4];
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
	using v3 = v<3, T>;

	template<typename T>
	using v4 = v<4, T>;

	template<uint32_t n, typename T>
	v<n, T> operator+(const v<n, T>& a, const v<n, T>& b);

	template<uint32_t n, typename T>
	v<n, T> operator-(const v<n, T>& a, const v<n, T>& b);

	template<uint32_t n, typename T>
	v<n, T> operator/(const v<n, T>& a, const v<n, T>& b);

	template<uint32_t n, typename T>
	v<n, T> operator*(const v<n, T>& a, const v<n, T>& b);

	template<uint32_t n, typename T>
	v<n, T>& operator+=(v<n, T>& a, const v<n, T>& b) {
		a = a + b;
		return a;
	}

	template<uint32_t n, typename T>
	v<n, T>& operator-=(v<n, T>& a, const v<n, T>& b) {
		a = a - b;
		return a;
	}

	template<uint32_t n, typename T>
	v<n, T>& operator/=(v<n, T>& a, const v<n, T>& b) {
		a = a / b;
		return a;
	}

	template<uint32_t n, typename T>
	v<n, T>& operator*=(v<n, T>& a, const v<n, T>& b) {
		a = a * b;
		return a;
	}

	template<uint32_t n, typename T>
	v<n, T> vFill(const T& val);

	template<uint32_t n, typename T>
	v<n, T> scale(const v<n, T>& a, const T& scaleFactor);

	template<uint32_t n, typename T>
	T dot(const v<n, T>& a, const v<n, T>& b);

	template<uint32_t n, typename T>
	T magnitude(const v<n, T>& a);

	template<uint32_t n, typename T>
	T length(const v<n, T>& a, const v<n, T>& b);

	template<uint32_t n, typename T>
	v<n, T> normalize(const v<n, T>& a);
}  // namespace pstd
