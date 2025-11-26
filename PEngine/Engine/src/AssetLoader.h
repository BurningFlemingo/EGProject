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

	struct MeshData {
		pstd::Array<pstd::Vec3> uniquePositions;
		pstd::Array<pstd::Vec2> uniqueUVs;

		pstd::Array<uint32_t> positionIndices;
		pstd::Array<uint32_t> uvIndices;
	};

	BMP loadBMP(pstd::Arena* pArena, const pstd::String path);
	MeshData loadOBJ(
		pstd::Arena* pArena, pstd::Arena scratchArena, const pstd::String path
	);
}  // namespace pstd
