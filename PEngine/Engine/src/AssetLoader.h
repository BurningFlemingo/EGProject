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
		uint32_t vertexCount;
		uint32_t indexCount;

		pstd::Vec3* pPositions;
		uint32_t* pIndices;
		pstd::Vec2* pUVs;
	};

	TextureData loadBMP(pstd::Arena* pArena, const pstd::String path);
	MeshData loadOBJ(
		pstd::Arena* pArena, pstd::Arena scratchArena, const pstd::String path
	);
}  // namespace Engine
