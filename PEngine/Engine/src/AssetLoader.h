#pragma once
#include "STD/PString.h"
#include "STD/PVector.h"
#include "STD/PArena.h"

namespace Engine {

	struct TextureData {
		uint32_t* pPixels;	// r8g8b8a8
		size_t width;
		size_t height;
	};

	struct MeshData {
		pstd::Array<pstd::Vec3> uniquePositions;
		pstd::Array<pstd::Vec2> uniqueUVs;

		pstd::Array<uint32_t> positionIndices;
		pstd::Array<uint32_t> uvIndices;
	};

	TextureData loadBMP(pstd::Arena* pArena, const pstd::String path);
	MeshData loadOBJ(
		pstd::Arena* pArena, pstd::Arena scratchArena, const pstd::String path
	);
}  // namespace Engine
