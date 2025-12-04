#pragma once
#include "STD/PString.h"
#include "STD/PVector.h"
#include "STD/PArena.h"

namespace Engine {

	// struct TextureData {
	// 	size_t width;
	// 	size_t height;
	// 	uint32_t* pPixels;	// r8g8b8a8
	// };

	// struct MeshData {
	// 	uint32_t vertexCount;
	// 	uint32_t indexCount;

	// 	pstd::Array<uint32_t> indices;
	// 	pstd::Array<pstd::Vec3> positions;
	// 	pstd::Array<pstd::Vec2> uvs;
	// };

	struct MeshHeader {
		enum MagicNumber : uint32_t { MAGIC = 0x4D455348 };
		enum VersionNumber : uint32_t { VERSION = 0 };
		uint32_t magic;
		uint32_t version;
		uint32_t vertexCount;
		uint32_t indexCount;
		alignas(8) uint8_t data[];
		// indices
		// positions
		// normals
		// tangents
		// uvs
	};

	struct TextureHeader {
		enum MagicNumber : uint32_t { MAGIC = 0x544558 };
		enum VersionNumber : uint32_t { VERSION = 0 };
		uint32_t magic;
		uint32_t version;
		uint32_t width;
		uint32_t height;
		alignas(8) uint8_t data[];
		// pixels
	};

	struct TextureData {
		size_t width;
		size_t height;
		uint32_t* pPixels;	// r8g8b8a8
	};

	struct MeshData {
		uint32_t vertexCount;
		uint32_t indexCount;

		uint32_t* pIndices;

		pstd::Vec3* pPositions;
		pstd::Vec3* pNormals;
		pstd::Vec3* pTangents;
		pstd::Vec2* pUVs;
	};

	TextureData loadTexture(pstd::Arena* pArena, const pstd::String path);
	MeshData loadMesh(pstd::Arena* pArena, const pstd::String path);
}  // namespace Engine
