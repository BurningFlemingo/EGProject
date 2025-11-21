#include "STD/PVector.h"
#include "STD/PMatrix.h"
#include "STD/PMath.h"

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
	// fix this
	Mat<n> res{};
	Mat<n> transposed{ pstd::calcTranspose(mat1) };
	for (uint32_t i{}; i < n; i++) {
		for (uint32_t j{}; j < n; j++) {
			res[i][j] = pstd::dot(transposed[j], mat2[i]);
		}
	}
	return res;
}

template<uint32_t n>
void pstd::scale(Mat<n>* mat, const Vec<n>& factor) {
	Mat<n> diagonalMatrix{ getIdentityMatrix<n>() * factor };
	*mat = (*mat) * diagonalMatrix;
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

Mat4 pstd::calcRotated(const Mat4& mat, const Rot3& rotor) {
	Mat4 res{ mat };
	rotate(&res, rotor);
	return res;
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

Mat4 pstd::calcPerspectiveMatrix(float fov, float ar, float n, float f) {
	Mat4 res{ .col1 = { .x = 1.f / (ar * pstd::tanf(fov / 2.f)),
						.y = 0.f,
						.z = 0.f,
						.w = 0.f },
			  .col2 = { .x = 0.f,
						.y = 1.f / (pstd::tanf(fov / 2.f)),
						.z = 0.f,
						.w = 0.f },
			  .col3 = { .x = 0.f, .y = 0.f, .z = f / (f - n), .w = 1.f },
			  .col4 = {
				  .x = 0.f, .y = 0.f, .z = -(f * n) / (f - n), .w = 0.f } };
	return res;
}

Mat4 pstd::calcLookAtMatrix(const Vec3& from, const Vec3& to, Vec3 up) {
	Vec3 z_basis{ pstd::calcNormalized(to - from) };
	Vec3 x_basis{ pstd::calcNormalized(pstd::cross(up, z_basis)) };
	Vec3 y_basis{ pstd::calcNormalized(pstd::cross(z_basis, x_basis)) };

	return Mat4{
		.col1{ .x = x_basis.x, .y = y_basis.x, .z = z_basis.x, .w = 0.0 },
		.col2{ .x = x_basis.y, .y = y_basis.y, .z = z_basis.y, .w = 0.0 },
		.col3{ .x = x_basis.z, .y = y_basis.z, .z = z_basis.z, .w = 0.0 },
		.col4{ .x = -dot(x_basis, from),
			   .y = -dot(y_basis, from),
			   .z = -dot(z_basis, from),
			   .w = 1.0 },
	};
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
