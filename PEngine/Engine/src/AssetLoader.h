#pragma once
#include "STD/PString.h"
#include "STD/PVector.h"
#include "STD/PArena.h"

namespace pstd {
	struct BMP {
		pstd::Vec4* pPixels;
		size_t width;
		size_t height;
	};

	struct OBJ {};

	BMP loadBMP(pstd::Arena* pArena, const char* path);
}  // namespace pstd
