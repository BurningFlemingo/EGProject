#include "public/PVector.h"
#include "public/PMath.h"

using namespace pstd;

template<uint32_t n, typename T>
v<n, T> pstd::operator+(const v<n, T>& a, const v<n, T>& b) {
	v<n, T> res{};
	for (uint32_t i{}; i < n; i++) {
		res.e[i] = a.e[i] + b.e[i];
	}
	return res;
}

template<uint32_t n, typename T>
v<n, T> pstd::operator-(const v<n, T>& a, const v<n, T>& b) {
	v<n, T> res{};
	for (uint32_t i{}; i < n; i++) {
		res.e[i] = a.e[i] - b.e[i];
	}
	return res;
}

template<uint32_t n, typename T>
v<n, T> pstd::operator/(const v<n, T>& a, const v<n, T>& b) {
	v<n, T> res{};
	for (uint32_t i{}; i < n; i++) {
		res.e[i] = a.e[i] / b.e[i];
	}
	return res;
}

template<uint32_t n, typename T>
v<n, T> pstd::vFill(const T& val) {
	v<n, T> res{};
	for (uint32_t i{}; i < n; i++) {
		res.e[i] = val;
	}
	return res;
}

template<uint32_t n, typename T>
v<n, T> pstd::scale(const v<n, T>& a, const T& factor) {
	v<n, T> res{ a };
	for (uint32_t i{}; i < n; i++) {
		res.e[i] *= factor;
	}
	return res;
}

template<uint32_t n, typename T>
T pstd::dot(const v<n, T>& a, const v<n, T>& b) {
	T res{};
	for (uint32_t i{}; i < n; i++) {
		res += a.e[i] * b.e[i];
	}
	return res;
}

template<typename T>
v3<T> pstd::cross(const v3<T>& a, const v3<T> b) {
	v3<T> res{ .x = a.y * b.z - a.z * b.y,	// yz
			   .y = -(a.x * b.z - a.z * b.x),  // xz
			   .z = a.x * b.y - a.y * b.x };  // xy
	return res;
}

template<uint32_t n, typename T>
T pstd::mag(const v<n, T>& a) {
	T res{};
	for (int i{}; i < n; i++) {
		res += a.e[i] * a.e[i];
	}
	res = pstd::sqrtfNewton<float>(res);
	return res;
}

template<uint32_t n, typename T>
T pstd::length(const v<n, T>& a, const v<n, T>& b) {
	T res{};
	v<n, T> difference{ b - a };
	res = pstd::mag(difference);
	return res;
}

template<uint32_t n, typename T>
v<n, T> pstd::normalize(const v<n, T>& a) {
	v<n, T> res{ a };
	res /= pstd::vFill<n>(pstd::mag(res));
	return res;
}

template<typename T>
rot3<T> pstd::createRotor(v3<T> a, v3<T> b) {
	// credit for these equations goes to https://jacquesheunis.com/post/rotors/
	a = pstd::normalize(a);
	b = pstd::normalize(b);

	// half way between a and b because rotors go double the angle
	v3<T> halfway{ pstd::normalize(a + b) };

	rot3<T> res{ .scalar = pstd::dot(halfway, a),
				 .xy = halfway.x * a.y - halfway.y * a.x,
				 .yz = halfway.y * a.z - halfway.z * a.y,
				 .zx = halfway.z * a.x - halfway.x * a.z };
	return res;
}

template<typename T>
rot3<T> pstd::createRotor(v3<T> a, v3<T> b, T radians) {
	a = pstd::normalize(a);
	b = pstd::normalize(b);
	radians *= 0.5;

	T cosAngle{ (T)pstd::cosfTaylor(radians) };
	T sinAngle{ (T)pstd::sinfTaylor(radians) };

	rot3<T> res{ .scalar = cosAngle,
				 .xy = b.x * a.y - b.y * a.x,
				 .yz = b.y * a.z - b.z * a.y,
				 .zx = b.z * a.x - b.x * a.z };
	res.xy *= sinAngle;
	res.yz *= sinAngle;
	res.zx *= sinAngle;

	return res;
}

template<typename T>
v3<T> pstd::rotate(const rot3<T>& r, const v3<T>& v) {
	// credit for these equations goes to https://jacquesheunis.com/post/rotors/
	T sX{ r.scalar * v.x + r.xy * v.y - r.zx * v.z };
	T sY{ r.scalar * v.y - r.xy * v.x + r.yz * v.z };
	T sZ{ r.scalar * v.z - r.yz * v.y + r.zx * v.x };
	T sXYZ{ r.xy * v.z + r.yz * v.x + r.zx * v.y };

	v3<T> res{ .x = sX * r.scalar + sY * r.xy + sXYZ * r.yz - sZ * r.zx,
			   .y = sY * r.scalar - sX * r.xy + sZ * r.yz + sXYZ * r.zx,
			   .z = sZ * r.scalar + sXYZ * r.xy - sY * r.yz + sX * r.zx };
	return res;
}

template<typename T>
rot3<T> pstd::composeRoters(rot3<T>& a, rot3<T>& b) {
	rot3<T> res{
		.scalar = a.scalar * b.scalar - a.xy * b.xy - a.yz * b.yz - a.zx * b.zx,
		.xy = a.scalar * b.xy + a.xy * b.scalar - a.yz * b.zx + a.zx * b.yz,
		.yz = a.scalar * b.yz + a.xy * b.zx + a.yz * b.scalar - a.zx * b.xy,
		.zx = a.scalar * b.zx - a.xy * b.yz + a.yz * b.xy + a.zx * b.scalar
	};
	return res;
}

#define INIT_FUNCTIONS(n, T)                                              \
	template v<n, T> pstd::operator+(const v<n, T>& a, const v<n, T>& b); \
	template v<n, T> pstd::operator-(const v<n, T>& a, const v<n, T>& b); \
	template v<n, T> pstd::operator/(const v<n, T>& a, const v<n, T>& b); \
	template v<n, T> pstd::vFill(const T& val);                           \
	template v<n, T> pstd::scale(const v<n, T>& a, const T& factor);      \
	template T pstd::dot(const v<n, T>& a, const v<n, T>& b);             \
	template T pstd::mag(const v<n, T>& a);                               \
	template T pstd::length(const v<n, T>& a, const v<n, T>& b);          \
	template v<n, T> pstd::normalize(const v<n, T>& a);

#define INIT_SINGLE_FUNCTIONS(n, T)                                      \
	template rot3<T> pstd::createRotor(v3<T> from, v3<T> to);            \
	template rot3<T> pstd::createRotor(v3<T> a, v3<T> b, T angle);       \
	template v3<T> pstd::rotate(const rot3<T>& rotor, const v3<T>& vec); \
	template rot3<T> pstd::composeRoters(rot3<T>& a, rot3<T>& b);        \
	template v3<T> pstd::cross(const v3<T>& a, const v3<T> b);

#define INIT_TYPES(func, n) \
	func(n, uint32_t);      \
	func(n, int32_t);       \
	func(n, uint64_t);      \
	func(n, int64_t);       \
	func(n, float);         \
	func(n, double);

INIT_TYPES(INIT_FUNCTIONS, 2)
INIT_TYPES(INIT_FUNCTIONS, 3)
INIT_TYPES(INIT_FUNCTIONS, 4)

INIT_TYPES(INIT_SINGLE_FUNCTIONS, 0)
