#include "public/PMatrix.h"

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
	res = (mat[0][0] * mat[1][1]) - (mat[0][1] * mat[0][1]);
	return res;
}

#define INIT_FUNCTIONS(n, T)                                                  \
	template v<n, T> pstd::operator*(const m<n, T>& mat, const v<n, T>& vec); \
	template m<n, T> pstd::operator*(const m<n, T>& mat1, m<n, T> mat2);      \
	template m<n, T> pstd::transpose(const m<n, T>& mat);                     \
	template void pstd::setDiagonal(m<n, T>* mat, const T& val);              \
	template m<n, T> pstd::getIdentityMatrix();

#define INIT_DET(n, T) template T pstd::det(const m<n, T>& mat);

#define INIT_TYPES(func, n)                                               \
	func(n, uint32_t) func(n, int32_t) func(n, uint64_t) func(n, int64_t) \
		func(n, float) func(n, double)

INIT_TYPES(INIT_FUNCTIONS, 2)
INIT_TYPES(INIT_FUNCTIONS, 3)
INIT_TYPES(INIT_FUNCTIONS, 4)
INIT_TYPES(INIT_DET, 2)
