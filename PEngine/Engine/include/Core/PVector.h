#pragma once
#include "PTypes.h"
#include "PAssert.h"

namespace pstd {
	template<uint32_t count>
	struct Vec;

	template<>
	struct Vec<2> {
		float& operator[](const size_t index) {
			ASSERT(index < 2);
			return e[index];
		}
		const float& operator[](const size_t index) const {
			ASSERT(index < 2);
			return e[index];
		}
		union {
			struct {
				float x;
				float y;
			};
			float e[2];
		};
	};

	template<>
	struct Vec<3> {
		float& operator[](const size_t index) {
			ASSERT(index < 3);
			return e[index];
		}
		const float& operator[](const size_t index) const {
			ASSERT(index < 3);
			return e[index];
		}
		union {
			struct {
				float x;
				float y;
				float z;
			};
			float e[3];
		};
	};

	template<>
	struct Vec<4> {
		float& operator[](const size_t index) {
			ASSERT(index < 4);
			return e[index];
		}
		const float& operator[](const size_t index) const {
			ASSERT(index < 4);
			return e[index];
		}
		union {
			struct {
				float x;
				float y;
				float z;
				float w;
			};
			float e[4];
		};
	};

	struct Rot3 {  // rotor
		float scalar;
		float xy;
		float yz;
		float zx;
	};

	using Vec2 = Vec<2>;
	using Vec3 = Vec<3>;
	using Vec4 = Vec<4>;

	constexpr Vec3 UP{ .y = 1.f };
	constexpr Vec3 RIGHT{ .x = 1.f };
	constexpr Vec3 FORWARD{ .z = 1.f };	 // vulkan points into the screen

	template<uint32_t n>
	Vec<n> operator+(const Vec<n>& a, const Vec<n>& b);

	template<uint32_t n>
	Vec<n> operator-(const Vec<n>& a, const Vec<n>& b);

	template<uint32_t n>
	Vec<n> operator/(const Vec<n>& a, const Vec<n>& b);

	template<uint32_t n>
	Vec<n>& operator+=(Vec<n>& a, const Vec<n>& b) {
		a = a + b;
		return a;
	}

	template<uint32_t n>
	Vec<n>& operator-=(Vec<n>& a, const Vec<n>& b) {
		a = a - b;
		return a;
	}

	template<uint32_t n>
	Vec<n>& operator/=(Vec<n>& a, const Vec<n>& b) {
		a = a / b;
		return a;
	}

	template<uint32_t n>
	constexpr Vec<n> getFilledVector(float val) {
		if constexpr (n == 2) {
			return Vec<n>{ .x = val, .y = val };
		}
		if constexpr (n == 3) {
			return Vec<n>{ .x = val, .y = val, .z = val };
		}
		if constexpr (n == 4) {
			return Vec<n>{ .x = val, .y = val, .z = val, .w = val };
		}
		return {};
	}

	template<uint32_t n>
	float dot(const Vec<n>& a, const Vec<n>& b);

	Vec3 cross(const Vec3& a, const Vec3 b);

	template<uint32_t n>
	Vec<n> hadamard(const Vec<n>& a, const Vec<n>& b);

	template<uint32_t n>
	float calcMagnitude(const Vec<n>& a);

	template<uint32_t n>
	float calcDistance(const Vec<n>& a, const Vec<n>& b);

	Rot3 calcRotor(Vec3 a, Vec3 b);	 // from a to b

	Rot3 calcRotor(Vec3 a, Vec3 b, float radians);	// from a to b

	template<uint32_t n>
	void scale(Vec<n>* a, const float& scaleFactor);

	template<uint32_t n>
	Vec<n> calcScaled(const Vec<n>& a, const float& scaleFactor) {
		Vec<n> res{ a };
		scale(&res, scaleFactor);
		return res;
	}

	template<uint32_t n>
	void normalize(Vec<n>* a);

	template<uint32_t n>
	Vec<n> calcNormalized(const Vec<n>& a) {
		Vec<n> res{ a };
		normalize(&res);
		return res;
	}

	void rotate(Vec3* Vec, const Rot3& rotor);

	inline Vec3 calcRotated(const Vec3& Vec, const Rot3& rotor) {
		Vec3 res{ Vec };
		rotate(&res, rotor);
		return res;
	}

	Rot3 composeRotor(const Rot3& a, const Rot3& b);

}  // namespace pstd
