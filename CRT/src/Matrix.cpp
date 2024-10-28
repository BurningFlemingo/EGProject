#include "PVector.h"
#include "public/PMatrix.h"
#include "public/PMath.h"

using namespace pstd;

template<uint32_t n>
Vec<n> pstd::operator*(const Mat<n>& mat, const Vec<n>& vec) {
	Vec<n> res{};
	for (uint32_t i{}; i < n; i++) {
		res[i] = pstd::dot(mat[i], vec);
	}
	return res;
}

template<uint32_t n>
Mat<n> pstd::operator*(const Mat<n>& mat1, const Mat<n>& mat2) {
	Mat<n> res{};
	Mat<n> transposed{ pstd::calcTranspose(mat1) };
	for (uint32_t i{}; i < n; i++) {
		for (uint32_t j{}; j < n; j++) {
			res[i][j] = pstd::dot(transposed[i], mat2[j]);
		}
	}
	return res;
}

template<uint32_t n>
void pstd::scale(Mat<n>* mat, const Vec<n>& factor) {
	for (uint32_t i{}; i < n; i++) {
		(*mat)[i] = pstd::hadamard((*mat)[i], factor);
	}
}

void pstd::rotate(Mat4* mat, const Rot3& rotor) {
	Vec3 basisX{ pstd::calcRotated(Vec3{ .x = 1 }, rotor) };
	Vec3 basisY{ pstd::calcRotated(Vec3{ .y = 1 }, rotor) };
	Vec3 basisZ{ pstd::calcRotated(Vec3{ .z = 1 }, rotor) };

	Mat4 rotMatrix{ .col1 = { .x = basisX.x, .y = basisX.y, .z = basisX.z },
					.col2 = { .x = basisY.x, .y = basisY.y, .z = basisY.z },
					.col3 = { .x = basisZ.x, .y = basisZ.y, .z = basisZ.z },
					.col4 = { .w = 1 } };
	*mat = rotMatrix * (*mat);
}

template<uint32_t n>
void pstd::transpose(Mat<n>* mat) {
	for (uint32_t row{}; row < n; row++) {
		for (uint32_t col{}; col < row; col++) {
			float tmp{ (*mat)[col][row] };
			(*mat)[col][row] = (*mat)[row][col];
			(*mat)[row][col] = tmp;
		}
	}
}

template<uint32_t n>
void pstd::setDiagonal(Mat<n>* mat, const float& val) {
	ASSERT(mat != nullptr);

	for (uint32_t i{}; i < n; i++) {
		(*mat)[i][i] = val;
	}
}

float pstd::calcDet(const Mat2& mat) {
	float res{};
	res = (mat[0][0] * mat[1][1]) - (mat[1][0] * mat[0][1]);
	return res;
}

Mat4 pstd::calcOrthoMatrix(
	float l, float r, float t, float b, float n, float f
) {
	Mat4 res{ .col1 = { .x = 2 / (r - l), .w = -(r + l) / (r - l) },
			  .col2 = { .y = 2 / (t - b), .w = -(t + b) / (t - b) },
			  .col3 = { .z = 1 / (f - n), .w = -n / (f - n) },
			  .col4 = { .w = 1 } };
	return res;
}

Mat4 pstd::calcPerspectiveMatrix(
	float n, float f, float aspectRatio, float fov
) {
	float t{ pstd::tanf(fov) * n };
	float b{ -t };
	float r{ t * aspectRatio };
	float l{ -r };

	Mat4 res{
		.col1 = { .x = (2 * n) / (r - l), .w = -(r + l) / (r - l) },
		.col2 = { .y = (2 * n) / (t - b), .w = -(t + b) / (t - b),},
		.col3 = { .z = 1 / (f - n), .w = -n / (f - n) }, 
		.col4 = {.z = 1}
	};
	return res;
}

Mat4 pstd::calcLookAtMatrix(const Vec3& from, const Vec3& to, Vec3 up) {
	Vec3 forward{ pstd::calcNormalized(to - from) };
	Vec3 right{ pstd::calcNormalized(pstd::cross(up, forward)) };
	up = pstd::cross(forward, right);
	Mat4 res{
		.col1{ .x = right.x, .y = up.x, .z = forward.x, .w = -from.x },
		.col2{ .x = right.y, .y = up.y, .z = forward.y, .w = -from.y },
		.col3{ .x = right.z, .y = up.z, .z = forward.z, .w = -from.z },
		.col4{ .w = 1 },
	};
	return res;
}

#define INIT_FUNCTIONS(n)                                                    \
	template Vec<n> pstd::operator*(const Mat<n>& mat, const Vec<n>& vec);   \
	template Mat<n> pstd::operator*(const Mat<n>& mat1, const Mat<n>& mat2); \
	template void pstd::scale(Mat<n>* mat, const Vec<n>& factor);            \
	template void pstd::transpose(Mat<n>* mat);                              \
	template void pstd::setDiagonal(Mat<n>* mat, const float& val);

INIT_FUNCTIONS(2)
INIT_FUNCTIONS(3)
INIT_FUNCTIONS(4)
