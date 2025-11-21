#pragma once
#include "STD/PString.h"
#include "STD/PVector.h"
#include "STD/PArena.h"

namespace pstd {
	struct BMP {
		// 32 Bit pixels
		uint32_t* pPixels;
		size_t width;
		size_t height;
	};

	struct OBJ {
		pstd::Array<pstd::Vec4> uniquePositions;
		pstd::Array<uint32_t> indices;
	};

	BMP loadBMP(pstd::Arena* pArena, const char* path);
	OBJ loadOBJ(
		pstd::Arena* pArena, pstd::Arena scratchArena, const char* path
	);
}  // namespace pstd
