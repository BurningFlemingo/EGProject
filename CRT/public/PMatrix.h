#pragma once
#include "PVector.h"

namespace pstd {
	template<uint32_t count>
	struct Mat;	 // column major ordered

	template<>
	struct Mat<2> {
		Vec2& operator[](const size_t index) {
			ASSERT(index < 2);
			return e[index];
		}
		const Vec2& operator[](const size_t index) const {
			ASSERT(index < 2);
			return e[index];
		}
		union {
			Vec2 e[2];
			struct {
				Vec2 col1;
				Vec2 col2;
			};
		};
	};

	template<>
	struct Mat<3> {
		Vec3& operator[](const size_t index) {
			ASSERT(index < 3);
			return e[index];
		}
		const Vec3& operator[](const size_t index) const {
			ASSERT(index < 3);
			return e[index];
		}
		union {
			Vec3 e[3];
			struct {
				Vec3 col1;
				Vec3 col2;
				Vec3 col3;
			};
		};
	};

	template<>
	struct Mat<4> {
		Vec4& operator[](const size_t index) {
			ASSERT(index < 4);
			return e[index];
		}
		const Vec4& operator[](const size_t index) const {
			ASSERT(index < 4);
			return e[index];
		}
		union {
			struct {
				Vec4 col1;
				Vec4 col2;
				Vec4 col3;
				Vec4 col4;
			};
			Vec4 e[4];
		};
	};

	using Mat2 = Mat<2>;
	using Mat3 = Mat<3>;
	using Mat4 = Mat<4>;

	template<uint32_t n>
	Vec<n> operator*(const Mat<n>& mat, const Vec<n>& vec);

	template<uint32_t n>
	Mat<n> operator*(const Mat<n>& mat1, const Mat<n>& mat2);

	template<uint32_t n>
	Mat<n>& operator*=(Mat<n>& mat1, const Mat<n>& mat2) {
		mat1 = mat1 * mat2;
		return mat1;
	}

	template<uint32_t n>
	constexpr Mat<n> getIdentityMatrix() {
		if constexpr (n == 2) {
			return Mat2 {
			.col1 = {.x = 1,}, 
			.col2 = {.y = 1}
		};
		}
		if constexpr (n == 3) {
			return Mat3{
			.col1 = {.x = 1,}, 
			.col2 = {.y = 1}, 
			.col3 = {.z = 1}
		};
		}
		if constexpr (n == 4) {
			return Mat4 {
			.col1 = {.x = 1,}, 
			.col2 = {.y = 1}, 
			.col3 = {.z = 1}, 
			.col4 = {.w = 1}, 
		};
		}
		return {};
	}

	template<uint32_t n>
	void scale(Mat<n>* mat, const Vec<n>& factor);

	template<uint32_t n>
	void scale(Mat<n>* mat, const float& factor) {
		Vec<n> scaleVec{ pstd::getFilledVector<n>() };
		scale(mat, scaleVec);
	}

	template<uint32_t n>
	Mat<n> calcScaled(const Mat<n>& mat, const Vec<n>& factor) {
		Mat<n> res{ mat };
		scale(&res, factor);
		return res;
	}

	template<uint32_t n>
	Mat<n> calcScaled(const Mat<n>& mat, float factor) {
		Mat<n> res{ mat };
		Vec<n> scaleVec{ pstd::getFilledVector<n>() };
		scale(&res, scaleVec);
		return res;
	}

	void translate(Mat4* mat, const Vec3& offset) {
		ASSERT(mat);
		mat->col1.w += offset.x;
		mat->col2.x += offset.y;
		mat->col3.w += offset.z;
	}

	Mat4 calcTranlated(const Mat4& mat, const Vec3& offset) {
		Mat4 res{ mat };
		translate(&res, offset);
		return res;
	}

	void rotate(Mat4* mat, const Rot3& rotor);

	template<uint32_t n>
	Mat<n> calcRotated(const Mat<n>& mat, const Rot3& rotor) {
		Mat<n> res{ mat };
		rotate(&res, rotor);
		return res;
	}

	template<uint32_t n>
	Mat<n> calcRotateMatrix(const Rot3& rotor) {
		Mat<n> res{ getIdentityMatrix<4>() };
		rotate(&res, rotor);
		return res;
	}

	template<uint32_t n>
	void transpose(Mat<n>* mat);

	template<uint32_t n>
	Mat<n> calcTranspose(const Mat<n>& mat) {
		Mat<n> res{ mat };
		transpose(&res);
		return res;
	}

	template<uint32_t n>
	void setDiagonal(Mat<n>* mat, const float& val);

	template<uint32_t n>
	Mat<n> calcDiagonalMatrix(const Mat<n>& mat, const float& Vecal) {
		Mat<n> res{ mat };
		setDiagonal(&res, Vecal);
		return res;
	}

	float calcDet(const Mat2& mat);

	Mat4 calcOrthoMatrix(
		float left, float right, float top, float bottom, float near, float far
	);

	Mat4 calcPerspectiveMatrix(
		float near, float far, float aspectRatio, float foVec
	);

	Mat4 calcLookAtMatrix(const Vec3& from, const Vec3& to, Vec3 up);

}  // namespace pstd
