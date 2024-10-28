#pragma once
#include "PVector.h"

namespace pstd {
	template<uint32_t count, typename T>
	struct m;

	template<typename T>
	struct m<2, T> {
		v<2, T>& operator[](const size_t index) {
			ASSERT(index < 2);
			v<2, T>& res{ e[index] };
			return res;
		}
		const v<2, T>& operator[](const size_t index) const {
			ASSERT(index < 2);
			const v<2, T>& res{ e[index] };
			return res;
		}
		union {
			v<2, T> e[2];
			struct {
				v<2, T> v1;
				v<2, T> v2;
			};
		};
	};

	template<typename T>
	struct m<3, T> {
		v<3, T>& operator[](const size_t index) {
			ASSERT(index < 3);
			v<3, T>& res{ e[index] };
			return res;
		}
		const v<3, T>& operator[](const size_t index) const {
			ASSERT(index < 3);
			const v<3, T>& res{ e[index] };
			return res;
		}
		union {
			v<3, T> e[3];
			struct {
				v<3, T> v1;
				v<3, T> v2;
				v<3, T> v3;
			};
		};
	};

	template<typename T>
	struct m<4, T> {
		v<4, T>& operator[](const size_t index) {
			ASSERT(index < 4);
			v<4, T>& res{ e[index] };
			return res;
		}
		const v<4, T>& operator[](const size_t index) const {
			ASSERT(index < 4);
			const v<4, T>& res{ e[index] };
			return res;
		}
		union {
			v<4, T> e[4];
			struct {
				v<4, T> v1;
				v<4, T> v2;
				v<4, T> v3;
				v<4, T> v4{ (T)1 };
			};
		};
	};

	template<typename T>
	using m2x2 = m<2, T>;

	template<typename T>
	using m3x3 = m<3, T>;

	template<typename T>
	using m4x4 = m<4, T>;

	template<uint32_t n, typename T>
	v<n, T> operator*(const m<n, T>& mat, const v<n, T>& vec);

	template<uint32_t n, typename T>
	m<n, T> operator*(const m<n, T>& mat1, m<n, T> mat2);

	template<uint32_t n, typename T>
	m<n, T> transpose(const m<n, T>& mat);

	template<uint32_t n, typename T>
	void setDiagonal(m<n, T>* mat, const T& val);

	template<uint32_t n, typename T>
	m<n, T> getIdentityMatrix();

	template<typename T>
	T det(const m<2, T>& mat);

	template<typename T>
	m<4, T> ortho(T left, T right, T top, T bottom, T near, T far);

	template<typename T>
	m<4, T> perspective(T near, T far, T aspectRatio, T fov);

	template<typename T>
	m<4, T> lookAt(const v<3, T>& from, const v<3, T>& to, v<3, T> up);

}  // namespace pstd
