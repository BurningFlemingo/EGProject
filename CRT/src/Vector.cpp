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
v<n, T> pstd::operator*(const v<n, T>& a, const v<n, T>& b) {
	v<n, T> res{};
	for (uint32_t i{}; i < n; i++) {
		res.e[i] = a.e[i] * b.e[i];
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

template<uint32_t n, typename T>
T pstd::magnitude(const v<n, T>& a) {
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
	res = pstd::magnitude(difference);
	return res;
}

template<uint32_t n, typename T>
v<n, T> pstd::normalize(const v<n, T>& a) {
	v<n, T> res{ a };
	res /= pstd::vFill<n>(pstd::magnitude(res));
	return res;
}

#define INIT_FUNCTIONS(n, T)                                              \
	template v<n, T> pstd::operator+(const v<n, T>& a, const v<n, T>& b); \
	template v<n, T> pstd::operator-(const v<n, T>& a, const v<n, T>& b); \
	template v<n, T> pstd::operator*(const v<n, T>& a, const v<n, T>& b); \
	template v<n, T> pstd::operator/(const v<n, T>& a, const v<n, T>& b); \
	template v<n, T> pstd::vFill(const T& val);                           \
	template v<n, T> pstd::scale(const v<n, T>& a, const T& factor);      \
	template T pstd::dot(const v<n, T>& a, const v<n, T>& b);             \
	template T pstd::magnitude(const v<n, T>& a);                         \
	template T pstd::length(const v<n, T>& a, const v<n, T>& b);          \
	template v<n, T> pstd::normalize(const v<n, T>& a);

#define INIT_TYPES(n)           \
	INIT_FUNCTIONS(n, uint32_t) \
	INIT_FUNCTIONS(n, int32_t)  \
	INIT_FUNCTIONS(n, uint64_t) \
	INIT_FUNCTIONS(n, int64_t)  \
	INIT_FUNCTIONS(n, float)    \
	INIT_FUNCTIONS(n, double)

INIT_TYPES(2)
INIT_TYPES(3)
INIT_TYPES(4)
