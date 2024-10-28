#pragma once
#include "PTypes.h"
#include "PAssert.h"

namespace pstd {

	template<uint32_t count, typename T>
	struct v;  // vector

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
	struct rot3 {  // rotor
		T scalar;
		T xy;
		T yz;
		T zx;
	};

	template<typename T>
	using v2 = v<2, T>;

	template<typename T>
	using v3 = v<3, T>;

	template<typename T>
	using v4 = v<4, T>;

	constexpr v3<float> UP{ .y = 1.f };
	constexpr v3<float> RIGHT{ .x = 1.f };
	constexpr v3<float> FORWARD{ .z = 1.f };  // vulkan points into the screen

	template<uint32_t n, typename T>
	v<n, T> operator+(const v<n, T>& a, const v<n, T>& b);

	template<uint32_t n, typename T>
	v<n, T> operator-(const v<n, T>& a, const v<n, T>& b);

	template<uint32_t n, typename T>
	v<n, T> operator/(const v<n, T>& a, const v<n, T>& b);

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

	template<typename T>
	v3<T> cross(const v3<T>& a, const v3<T> b);

	template<uint32_t n, typename T>
	T mag(const v<n, T>& a);

	template<uint32_t n, typename T>
	T length(const v<n, T>& a, const v<n, T>& b);

	template<uint32_t n, typename T>
	v<n, T> normalize(const v<n, T>& a);

	template<typename T>
	rot3<T> createRotor(v3<T> a, v3<T> b);	// from a to b

	template<typename T>
	rot3<T> createRotor(v3<T> a, v3<T> b, T radians);  // from a to b

	template<typename T>
	v3<T> rotate(const rot3<T>& rotor, const v3<T>& vec);

	template<typename T>
	rot3<T> composeRoters(rot3<T>& a, rot3<T>& b);

}  // namespace pstd
