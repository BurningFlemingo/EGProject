#include "PVector.h"
#include "public/PMatrix.h"
#include "public/PMath.h"

using namespace pstd;

template<uint32_t n, typename T>
v<n, T> pstd::operator*(const m<n, T>& mat, const v<n, T>& vec) {
	v<n, T> res{};
	for (uint32_t i{}; i < n; i++) {
		res[i] = pstd::dot(mat[i], vec);
	}
	return res;
}

template<uint32_t n, typename T>
m<n, T> transpose(const m<n, T>& mat) {
	m<n, T> res{};
	for (uint32_t row{}; row < n; row++) {
		for (uint32_t col{}; col < n; col++) {
			res[col][row] = mat[row][col];
		}
	}
	return res;
}

template<uint32_t n, typename T>
m<n, T> pstd::operator*(const m<n, T>& mat1, m<n, T> mat2) {
	m<n, T> res{};
	mat2 = pstd::transpose(mat2);
	for (uint32_t i{}; i < n; i++) {
		for (uint32_t j{}; j < n; j++) {
			res[i][j] = pstd::dot(mat1[i], mat2[j]);
		}
	}
	return res;
}

template<uint32_t n, typename T>
void pstd::setDiagonal(m<n, T>* mat, const T& val) {
	ASSERT(mat != nullptr);

	for (uint32_t i{}; i < n; i++) {
		(*mat)[i][i] = val;
	}
}

template<uint32_t n, typename T>
m<n, T> pstd::getIdentityMatrix() {
	m<n, T> res{};
	pstd::setDiagonal(&res, (T)1);
	return res;
}

template<typename T>
T pstd::det(const m<2, T>& mat) {
	T res{};
	res = (mat[0][0] * mat[1][1]) - (mat[1][0] * mat[0][1]);
	return res;
}

template<typename T>
m<4, T> pstd::ortho(T l, T r, T t, T b, T n, T f) {
	m<4, T> res{ .v1 = { .x = 2 / (r - l), .w = -(r + l) / (r - l) },
				 .v2 = { .y = 2 / (t - b), .w = -(t + b) / (t - b) },
				 .v3 = { .z = 1 / (f - n), .w = -n / (f - n) },
				 .v4 = { .w = 1 } };
	return res;
}

template<typename T>
m<4, T> pstd::perspective(T n, T f, T aspectRatio, T fov) {
	T t{ (T)pstd::tanfTaylor<T>(fov) * n };
	T b{ -t };
	T r{ t * aspectRatio };
	T l{ -r };

	m<4, T> res{
		.v1 = { .x = (2 * n) / (r - l), .w = -(r + l) / (r - l) },
		.v2 = { .y = (2 * n) / (t - b), .w = -(t + b) / (t - b),},
		.v3 = { .z = 1 / (f - n), .w = -n / (f - n) }, 
		.v4 = {.z = 1}
	};
	return res;
}

template<typename T>
m<4, T> pstd::lookAt(const v<3, T>& from, const v<3, T>& to, v<3, T> up) {
	v<3, T> forward{ pstd::normalize(to - from) };
	v<3, T> right{ pstd::normalize(pstd::cross(up, forward)) };
	up = pstd::cross(forward, right);
	m<4, T> res{
		.v1{ .x = right.x, .y = right.y, .z = right.z, .w = -from.x },
		.v2{ .x = up.x, .y = up.y, .z = up.z, .w = -from.y },
		.v3{ .x = forward.x, .y = forward.y, .z = forward.z, .w = -from.z },
		.v4{ .w = 1 },
	};
	return res;
}

#define INIT_FUNCTIONS(n, T)                                                  \
	template v<n, T> pstd::operator*(const m<n, T>& mat, const v<n, T>& vec); \
	template m<n, T> pstd::operator*(const m<n, T>& mat1, m<n, T> mat2);      \
	template m<n, T> pstd::transpose(const m<n, T>& mat);                     \
	template void pstd::setDiagonal(m<n, T>* mat, const T& val);              \
	template m<n, T> pstd::getIdentityMatrix();

#define INIT_SINGLE_FUNCTIONS(n, T)                                          \
	template T pstd::det(const m<2, T>& mat);                                \
	template m<4, T> pstd::ortho(                                            \
		T left, T right, T top, T bottom, T near, T far                      \
	);                                                                       \
	template m<4, T> pstd::perspective(T near, T far, T aspectRatio, T fov); \
	template m<4, T> pstd::lookAt(                                           \
		const v<3, T>& from, const v<3, T>& to, v<3, T> up                   \
	);

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
