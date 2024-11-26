#include "include/PVector.h"
#include "include/PMath.h"

using namespace pstd;

template<uint32_t n>
Vec<n> pstd::operator+(const Vec<n>& a, const Vec<n>& b) {
	Vec<n> res{};
	for (uint32_t i{}; i < n; i++) {
		res.e[i] = a.e[i] + b.e[i];
	}
	return res;
}

template<uint32_t n>
Vec<n> pstd::operator-(const Vec<n>& a, const Vec<n>& b) {
	Vec<n> res{};
	for (uint32_t i{}; i < n; i++) {
		res.e[i] = a.e[i] - b.e[i];
	}
	return res;
}

template<uint32_t n>
Vec<n> pstd::operator/(const Vec<n>& a, const Vec<n>& b) {
	Vec<n> res{};
	for (uint32_t i{}; i < n; i++) {
		res.e[i] = a.e[i] / b.e[i];
	}
	return res;
}

template<uint32_t n>
float pstd::dot(const Vec<n>& a, const Vec<n>& b) {
	float res{};
	for (uint32_t i{}; i < n; i++) {
		res += a.e[i] * b.e[i];
	}
	return res;
}

Vec3 pstd::cross(const Vec3& a, const Vec3 b) {
	Vec3 res{ .x = a.y * b.z - a.z * b.y,  // yz
			  .y = -(a.x * b.z - a.z * b.x),  // xz
			  .z = a.x * b.y - a.y * b.x };	 // xy
	return res;
}

template<uint32_t n>
Vec<n> pstd::hadamard(const Vec<n>& a, const Vec<n> b) {
	Vec<n> res{ a };
	for (uint32_t i{}; i < n; i++) {
		res[i] *= b[i];
	}
	return res;
}

template<uint32_t n>
float pstd::calcMagnitude(const Vec<n>& a) {
	float res{};
	for (int i{}; i < n; i++) {
		res += a.e[i] * a.e[i];
	}
	res = (float)pstd::sqrtf((float)res);
	return res;
}

template<uint32_t n>
float pstd::calcDistance(const Vec<n>& a, const Vec<n>& b) {
	float res{};
	Vec<n> difference{ b - a };
	res = pstd::calcMagnitude(difference);
	return res;
}

Rot3 pstd::calcRotor(Vec3 a, Vec3 b) {
	// credit for these equations goes to https://jacquesheunis.com/post/rotors/
	pstd::normalize(&a);
	pstd::normalize(&b);

	// half way between a and b because rotors rotate by double the angle
	// between a and b
	Vec3 halfway{ pstd::calcNormalized(a + b) };

	Rot3 res{ .scalar = pstd::dot(halfway, a),
			  .xy = halfway.x * a.y - halfway.y * a.x,
			  .yz = halfway.y * a.z - halfway.z * a.y,
			  .zx = halfway.z * a.x - halfway.x * a.z };
	return res;
}

Rot3 pstd::calcRotor(Vec3 a, Vec3 b, float radians) {
	// credit for these equations goes to https://jacquesheunis.com/post/rotors/
	pstd::normalize(&a);
	pstd::normalize(&b);
	radians *= 0.5;

	float cosAngle{ pstd::cosf(radians) };
	float sinAngle{ pstd::sinf(radians) };

	Rot3 res{ .scalar = cosAngle,
			  .xy = b.x * a.y - b.y * a.x,
			  .yz = b.y * a.z - b.z * a.y,
			  .zx = b.z * a.x - b.x * a.z };
	res.xy *= sinAngle;
	res.yz *= sinAngle;
	res.zx *= sinAngle;

	return res;
}

template<uint32_t n>
void pstd::scale(Vec<n>* a, const float& factor) {
	ASSERT(a);

	for (uint32_t i{}; i < n; i++) {
		a->e[i] *= factor;
	}
}

template<uint32_t n>
void pstd::normalize(Vec<n>* a) {
	ASSERT(a);

	*a /= pstd::getFilledVector<n>(pstd::calcMagnitude(*a));
}

void pstd::rotate(Vec3* vPtr, const Rot3& r) {
	ASSERT(vPtr);

	// credit for these equations goes to https://jacquesheunis.com/post/rotors/
	const Vec3& v{ *vPtr };
	float sX{ r.scalar * v.x + r.xy * v.y - r.zx * v.z };
	float sY{ r.scalar * v.y - r.xy * v.x + r.yz * v.z };
	float sZ{ r.scalar * v.z - r.yz * v.y + r.zx * v.x };
	float sXYZ{ r.xy * v.z + r.yz * v.x + r.zx * v.y };

	*vPtr = Vec3{ .x = sX * r.scalar + sY * r.xy + sXYZ * r.yz - sZ * r.zx,
				  .y = sY * r.scalar - sX * r.xy + sZ * r.yz + sXYZ * r.zx,
				  .z = sZ * r.scalar + sXYZ * r.xy - sY * r.yz + sX * r.zx };
}

Rot3 pstd::composeRotor(Rot3& a, Rot3& b) {
	Rot3 res{
		.scalar = a.scalar * b.scalar - a.xy * b.xy - a.yz * b.yz - a.zx * b.zx,
		.xy = a.scalar * b.xy + a.xy * b.scalar - a.yz * b.zx + a.zx * b.yz,
		.yz = a.scalar * b.yz + a.xy * b.zx + a.yz * b.scalar - a.zx * b.xy,
		.zx = a.scalar * b.zx - a.xy * b.yz + a.yz * b.xy + a.zx * b.scalar
	};
	return res;
}

#define INIT_FUNCTIONS(n)                                                \
	template Vec<n> pstd::operator+(const Vec<n>& a, const Vec<n>& b);   \
	template Vec<n> pstd::operator-(const Vec<n>& a, const Vec<n>& b);   \
	template Vec<n> pstd::operator/(const Vec<n>& a, const Vec<n>& b);   \
	template float pstd::dot(const Vec<n>& a, const Vec<n>& b);          \
	template float pstd::calcMagnitude(const Vec<n>& a);                 \
	template float pstd::calcDistance(const Vec<n>& a, const Vec<n>& b); \
	template void pstd::scale(Vec<n>* a, const float& factor);           \
	template void pstd::normalize(Vec<n>* a);                            \
	template Vec<n> pstd::hadamard(const Vec<n>& a, const Vec<n> b);

INIT_FUNCTIONS(2) INIT_FUNCTIONS(3) INIT_FUNCTIONS(4)
