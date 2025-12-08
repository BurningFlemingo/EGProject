#include "AssetLoader.h"
#include "Logging.h"
#include "STD/PArray.h"
#include "STD/PFileIO.h"
#include "STD/PAlgorithm.h"
#include "STD/PString.h"
#include "STD/PHashMap.h"
#include <new>

namespace {

#pragma pack(push, 1)
	struct BMPHeader {
		uint16_t fileType;
		uint32_t fileSize;
		uint16_t reserved1;
		uint16_t reserved2;
		uint32_t pxOffset;
		uint32_t headerSize;
		int32_t pxWidth;
		int32_t pxHeight;
		uint16_t nPlanes;
		uint16_t bitsPerPixel;
		uint32_t compressionMethod;
		uint32_t bmpSize;
		int32_t horizontalResolution;
		int32_t verticalResolution;
		uint32_t nColorsUsed;
		uint32_t nColorsImportant;	// generally ignored

		uint32_t redMask;
		uint32_t greenMask;
		uint32_t blueMask;
	};
#pragma pack(pop)

	struct OBJ {
		pstd::Array<pstd::Vec3> uniquePositions;
		pstd::Array<pstd::Vec3> uniqueNormals;
		pstd::Array<pstd::Vec2> uniqueUVs;

		pstd::Array<uint32_t> positionIndices;
		pstd::Array<uint32_t> normalIndices;
		pstd::Array<uint32_t> uvIndices;
	};

	struct OBJMetadata {
		uint32_t positionCount;
		uint32_t normalCount;
		uint32_t uvCount;
		uint32_t vertexCount;
	};
	OBJ parseOBJ(
		pstd::Arena* pArena, pstd::Arena scratchArena, pstd::String lines
	);
	OBJMetadata parseOBJMetadata(pstd::String objString);
	void parseFace(
		const pstd::Span<pstd::String>& contents,
		pstd::Arena scratchArena,
		pstd::Array<uint32_t>* pPositionIndices,
		pstd::Array<uint32_t>* pNormalIndices,
		pstd::Array<uint32_t>* pUVIndices
	);

	void createDirectories(
		pstd::Arena scratchArena, pstd::String nestedDirectorys
	);

	pstd::Vec3 calcTangent(
		pstd::Span<pstd::Vec3> positions, pstd::Span<pstd::Vec2> uvs
	);

}  // namespace

void cookBMP(pstd::Arena scratchArena, const pstd::String path) {
	pstd::Allocation rawBMP{ pstd::readFile(&scratchArena, path) };

	ASSERT(rawBMP.size != 0);
	ASSERT(rawBMP.block != nullptr);

	BMPHeader* header{ rcast<BMPHeader*>(rawBMP.block) };

	ASSERT(header->pxWidth > 0);
	ASSERT(header->pxHeight > 0);
	ASSERT(header->bitsPerPixel == 32 || header->bitsPerPixel == 24);
	ASSERT(header->compressionMethod == 3 || header->compressionMethod == 0);

	uint32_t absWidth{ ncast<uint32_t>(pstd::abs(header->pxWidth)) };
	uint32_t absHeight{ ncast<uint32_t>(pstd::abs(header->pxHeight)) };

	size_t nPixels{ absHeight * absWidth };

	uint32_t redMask{};
	uint32_t greenMask{};
	uint32_t blueMask{};
	uint32_t alphaMask{};

	if (header->compressionMethod == 0) {
		redMask = 0x000000FF;
		greenMask = 0x0000FF00;
		blueMask = 0x00FF0000;
		alphaMask = 0xFF000000;
	} else {
		redMask = header->redMask;
		greenMask = header->greenMask;
		blueMask = header->blueMask;
		alphaMask = ~(redMask | greenMask | blueMask);
	}

	uint8_t* pPixels{ rcast<uint8_t*>(rawBMP.block) + header->pxOffset };

	pstd::FirstSetBit redShift{ pstd::bitscanForward(redMask) };
	pstd::FirstSetBit greenShift{ pstd::bitscanForward(greenMask) };
	pstd::FirstSetBit blueShift{ pstd::bitscanForward(blueMask) };
	pstd::FirstSetBit alphaShift{ pstd::bitscanForward(alphaMask) };

	ASSERT(redShift.found);
	ASSERT(greenShift.found);
	ASSERT(blueShift.found);
	ASSERT(alphaShift.found);

	size_t initialArenaOffset{ scratchArena.offset };

	auto* textureHeader{ pstd::alloc<Engine::TextureHeader>(&scratchArena) };
	auto* pixelArray{ pstd::alloc<uint32_t>(&scratchArena, nPixels, 8) };

	size_t textureFileSize{ scratchArena.offset - initialArenaOffset };

	textureHeader->magic = Engine::TextureHeader::MAGIC;
	textureHeader->version = Engine::TextureHeader::VERSION;
	textureHeader->width = absWidth;
	textureHeader->height = absHeight;

	// BMP pixels are aligned to 4byte boundarys
	size_t stride{ absWidth * (header->bitsPerPixel / 8) };
	bool topDown{ header->pxHeight < 0 };
	stride = (stride + 3) & ~3;
	for (size_t dstY{}; dstY < absHeight; dstY++) {
		size_t srcY{ dstY };
		if (topDown) {
			srcY = absHeight - dstY - 1;
		}

		for (size_t x{}; x < absWidth; x++) {
			size_t colorByteIndex{ (srcY * stride) +
								   (x * (header->bitsPerPixel / 8)) };
			uint32_t color{ *rcast<uint32_t*>(pPixels + colorByteIndex) };
			if (header->bitsPerPixel == 24) {
				color = color | 0xFF << 24;
			}

			pixelArray[(dstY * absWidth) + x] =
				(((color >> redShift.shift) & 0xFF) << 0) |
				(((color >> greenShift.shift) & 0xFF) << 8) |
				(((color >> blueShift.shift) & 0xFF) << 16) |
				(((color >> alphaShift.shift) & 0xFF) << 24);
		}
	}
	uint32_t r{ pixelArray[0] & 0xFF };
	uint32_t g{ pixelArray[0] >> 8 & 0xFF };
	uint32_t b{ pixelArray[0] >> 16 & 0xFF };
	uint32_t a{ pixelArray[0] >> 24 & 0xFF };
	LOG_INFO("(%u, %u, %u, %u)\n", r, g, b, a);

	pstd::String texturePath{ path };
	pstd::readLastToken(&texturePath, ".");
	pstd::readToken(
		&texturePath, "\\/"
	);	// to remove leading 'assets' directory

	texturePath =
		pstd::makeConcatted(&scratchArena, "generated\\", texturePath);
	texturePath = pstd::makeConcatted(&scratchArena, texturePath, "texture");

	createDirectories(scratchArena, texturePath);

	pstd::FileHandle handle{ pstd::openFile(
		pstd::createCString(&scratchArena, texturePath),
		pstd::FileAccess::write,
		pstd::FileAccess::none,
		pstd::FileCreate::createAlways
	) };
	pstd::writeFile(handle, textureHeader, textureFileSize);
	pstd::closeFile(handle);
}

void cookOBJ(
	pstd::Arena primaryScratchArena,
	pstd::Arena secondaryScratchArena,
	const pstd::String path
) {
	pstd::String lines{
		pstd::createString(pstd::readFile(&primaryScratchArena, path))
	};

	OBJ obj{ parseOBJ(&primaryScratchArena, secondaryScratchArena, lines) };

	uint32_t indexCount{
		ncast<uint32_t>(max(obj.positionIndices.count, obj.uvIndices.count))
	};
	uint32_t vertexCount{ indexCount };

	size_t initialArenaOffset{ primaryScratchArena.offset };

	auto* pMeshHeader{ pstd::alloc<Engine::MeshHeader>(&primaryScratchArena) };
	auto* pIndices{
		pstd::alloc<uint32_t>(&primaryScratchArena, indexCount, 8)
	};
	auto* pPositions{
		pstd::alloc<pstd::Vec3>(&primaryScratchArena, indexCount, 8)
	};
	auto* pNormals{
		pstd::alloc<pstd::Vec3>(&primaryScratchArena, indexCount, 8)
	};
	auto* pTangents{
		pstd::alloc<pstd::Vec3>(&primaryScratchArena, indexCount, 8)
	};

	auto* pUVs{ pstd::alloc<pstd::Vec2>(&primaryScratchArena, indexCount, 8) };

	size_t meshFileSize{ primaryScratchArena.offset - initialArenaOffset };

	pMeshHeader->magic = Engine::MeshHeader::MAGIC;
	pMeshHeader->version = Engine::MeshHeader::VERSION;
	pMeshHeader->indexCount = indexCount;
	pMeshHeader->vertexCount = vertexCount;

	for (uint32_t i{}; i < indexCount; i++) {
		pIndices[i] = i;
	}
	for (uint32_t i{}; i < indexCount; i++) {
		pPositions[i] = obj.uniquePositions[obj.positionIndices[i]];
	}
	for (uint32_t i{}; i < indexCount; i++) {
		pNormals[i] = obj.uniqueNormals[obj.normalIndices[i]];
	}
	for (uint32_t i{}; i < indexCount; i++) {
		pUVs[i] = obj.uniqueUVs[obj.uvIndices[i]];
	}

	for (size_t i{}; i < indexCount - 2; i += 3) {
		// TODO: fix this, but not till we need normal maps.
		ASSERT(pNormals[pIndices[i]] == pNormals[pIndices[i + 1]]);
		ASSERT(pNormals[pIndices[i]] == pNormals[pIndices[i + 2]]);

		pstd::Vec3 p1{ pPositions[pIndices[i]] };
		pstd::Vec3 p2{ pPositions[pIndices[i + 1]] };
		pstd::Vec3 p3{};

		pstd::StaticArray<pstd::Vec3, 3> positions{
			pPositions[pIndices[i]],
			pPositions[pIndices[i + 1]],
			pPositions[pIndices[i + 2]],
		};

		pstd::StaticArray<pstd::Vec2, 3> uvs{
			pUVs[pIndices[i]],
			pUVs[pIndices[i + 1]],
			pUVs[pIndices[i + 2]],
		};

		pstd::Vec3 normal{ pNormals[pIndices[i]] };

		pstd::Vec3 tangent{ pstd::calcNormalized(calcTangent(positions, uvs)) };
		// gram-schmidt proccess / re-orthonormalization
		tangent = pstd::calcNormalized(
			tangent - (normal * pstd::dot(normal, tangent))
		);

		LOG_INFO("tangent: (%f, %f, %f)\n", tangent.x, tangent.y, tangent.z);
		LOG_INFO("normal: (%f, %f, %f)\n", normal.x, normal.y, normal.z);
		LOG_INFO("\n");

		pTangents[i] = tangent;
		pTangents[i + 1] = tangent;
		pTangents[i + 2] = tangent;
	}

	pstd::String meshPath{ path };
	pstd::readLastToken(&meshPath, ".");
	pstd::readToken(&meshPath, "\\/");	// to remove leading 'assets' directory

	meshPath =
		pstd::makeConcatted(&primaryScratchArena, "generated\\", meshPath);
	meshPath = pstd::makeConcatted(&primaryScratchArena, meshPath, "mesh");

	createDirectories(primaryScratchArena, meshPath);

	pstd::FileHandle handle{ pstd::openFile(
		pstd::createCString(&primaryScratchArena, meshPath),
		pstd::FileAccess::write,
		pstd::FileAccess::none,
		pstd::FileCreate::createAlways
	) };
	pstd::writeFile(handle, pMeshHeader, meshFileSize);
	pstd::closeFile(handle);
}

namespace {

	OBJMetadata parseOBJMetadata(pstd::String objString) {
		uint32_t positionCount{};
		uint32_t normalCount{};
		uint32_t uvCount{};
		uint32_t vertexCount{};
		while (objString.size > 0) {
			pstd::String line{ pstd::readLine(&objString) };
			pstd::String identifier{ pstd::readToken(&line) };

			if (identifier == "v") {
				positionCount++;
			} else if (identifier == "vt") {
				uvCount++;
			} else if (identifier == "vn") {
				normalCount++;
			} else if (identifier == "f") {
				size_t ngon{ pstd::countTokens(line, " ") };

				ASSERT(
					ngon == 3 || ngon == 4,
					"obj loader only supports tris and quads"
				);

				if (ngon == 3) {
					vertexCount += 3;
				} else {
					vertexCount += 6;
				}
			}
		}

		return OBJMetadata{ .positionCount = positionCount,
							.normalCount = normalCount,
							.uvCount = uvCount,
							.vertexCount = vertexCount };
	}

	void parseFace(
		const pstd::Span<pstd::String>& contents,
		pstd::Arena scratchArena,
		pstd::Array<uint32_t>* pPositionIndices,
		pstd::Array<uint32_t>* pNormalIndices,
		pstd::Array<uint32_t>* pUVIndices
	) {
		ASSERT(contents.count <= 4);

		uint32_t positionIndices[4];
		uint32_t normalIndices[4];
		uint32_t uvIndices[4];

		for (size_t i{}; i < contents.count; i++) {
			pstd::Array<pstd::String> face{
				pstd::split(&scratchArena, contents[i], 3, "/")
			};

			ASSERT(face.count == 3);

			positionIndices[i] = pstd::parse<uint32_t>(face[0]);
			normalIndices[i] = pstd::parse<uint32_t>(face[2]);
			uvIndices[i] = pstd::parse<uint32_t>(face[1]);
		}

		if (contents.count == 3) {
			// -1 because obj is 1 indexed, front faces are assumed to be CCW
			pstd::pushBack(pPositionIndices, positionIndices[0] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[1] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[2] - 1);

			pstd::pushBack(pNormalIndices, normalIndices[0] - 1);
			pstd::pushBack(pNormalIndices, normalIndices[1] - 1);
			pstd::pushBack(pNormalIndices, normalIndices[2] - 1);

			pstd::pushBack(pUVIndices, uvIndices[0] - 1);
			pstd::pushBack(pUVIndices, uvIndices[1] - 1);
			pstd::pushBack(pUVIndices, uvIndices[2] - 1);
		} else {
			pstd::pushBack(pPositionIndices, positionIndices[0] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[1] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[2] - 1);

			pstd::pushBack(pPositionIndices, positionIndices[0] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[2] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[3] - 1);

			pstd::pushBack(pNormalIndices, normalIndices[0] - 1);
			pstd::pushBack(pNormalIndices, normalIndices[1] - 1);
			pstd::pushBack(pNormalIndices, normalIndices[2] - 1);

			pstd::pushBack(pNormalIndices, normalIndices[0] - 1);
			pstd::pushBack(pNormalIndices, normalIndices[2] - 1);
			pstd::pushBack(pNormalIndices, normalIndices[3] - 1);

			pstd::pushBack(pUVIndices, uvIndices[0] - 1);
			pstd::pushBack(pUVIndices, uvIndices[1] - 1);
			pstd::pushBack(pUVIndices, uvIndices[2] - 1);

			pstd::pushBack(pUVIndices, uvIndices[0] - 1);
			pstd::pushBack(pUVIndices, uvIndices[2] - 1);
			pstd::pushBack(pUVIndices, uvIndices[3] - 1);
		}
	}

	OBJ parseOBJ(
		pstd::Arena* pArena, pstd::Arena scratchArena, pstd::String lines
	) {
		OBJMetadata meta{ parseOBJMetadata(lines) };

		OBJ obj{
			.uniquePositions =
				pstd::createArray<pstd::Vec3>(pArena, meta.positionCount, 0),
			.uniqueNormals =
				pstd::createArray<pstd::Vec3>(pArena, meta.normalCount, 0),
			.uniqueUVs = pstd::createArray<pstd::Vec2>(pArena, meta.uvCount, 0),
			.positionIndices =
				pstd::createArray<uint32_t>(pArena, meta.vertexCount, 0),
			.normalIndices =
				pstd::createArray<uint32_t>(pArena, meta.vertexCount, 0),
			.uvIndices =
				pstd::createArray<uint32_t>(pArena, meta.vertexCount, 0)
		};

		while (lines.size > 0) {
			pstd::String line{ pstd::readLine(&lines) };
			pstd::String identifier{ pstd::readToken(&line) };

			pstd::Array<pstd::String> contents{
				pstd::split(&scratchArena, line, 4)
			};

			if (identifier == "f") {
				parseFace(
					contents,
					scratchArena,
					&obj.positionIndices,
					&obj.normalIndices,
					&obj.uvIndices
				);
			} else if (identifier == "v") {
				pstd::Vec3 position{ pstd::parse<float>(contents[0]),
									 pstd::parse<float>(contents[1]),
									 pstd::parse<float>(contents[2]) };
				pstd::pushBack(&obj.uniquePositions, position);
			} else if (identifier == "vn") {
				pstd::Vec3 normal{ pstd::parse<float>(contents[0]),
								   pstd::parse<float>(contents[1]),
								   pstd::parse<float>(contents[2]) };
				pstd::pushBack(&obj.uniqueNormals, normal);
			} else if (identifier == "vt") {
				pstd::Vec2 uv{ pstd::parse<float>(contents[0]),
							   pstd::parse<float>(contents[1]) };
				pstd::pushBack(&obj.uniqueUVs, uv);
			}
		}

		return obj;
	}

	void createDirectories(
		pstd::Arena scratchArena, pstd::String nestedDirectorys
	) {
		for (size_t i{}; i < nestedDirectorys.size; i++) {
			if (nestedDirectorys[i] == '\\') {
				pstd::String path(nestedDirectorys.buffer, i);
				pstd::createDirectory(pstd::createCString(&scratchArena, path));
			}
		}
	}

	pstd::Vec3 calcTangent(
		pstd::Span<pstd::Vec3> positions, pstd::Span<pstd::Vec2> uvs
	) {
		pstd::Vec3 e1{ positions[0] - positions[1] };
		pstd::Vec3 e2{ positions[2] - positions[1] };
		pstd::Vec2 duv1{ uvs[0] - uvs[1] };
		pstd::Vec2 duv2{ uvs[2] - uvs[1] };

		float f{ 1.f / ((duv1.x * duv2.y) - (duv2.x * duv1.y)) };

		return pstd::Vec3{
			(duv2.y * e1.x) - (duv1.y * e2.x),
			(duv2.y * e1.y) - (duv1.y * e2.y),
			(duv2.y * e1.z) - (duv1.y * e2.z),
		} * f;
	}
}  // namespace
