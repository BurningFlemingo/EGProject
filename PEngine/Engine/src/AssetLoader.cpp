#include "AssetLoader.h"
#include "Logging.h"
#include "STD/PArray.h"
#include "STD/PFileIO.h"
#include "STD/PAlgorithm.h"
#include "STD/PString.h"

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
		pstd::Array<pstd::Vec2> uniqueUVs;

		pstd::Array<uint32_t> positionIndices;
		pstd::Array<uint32_t> uvIndices;
	};

	struct OBJMetadata {
		uint32_t positionCount;
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
		pstd::Array<uint32_t>* pUVIndices
	);

}  // namespace

Engine::TextureData
	Engine::loadBMP(pstd::Arena* pArena, const pstd::String path) {
	pstd::Allocation rawBMP{ pstd::readFile(pArena, path) };

	ASSERT(rawBMP.size != 0);
	ASSERT(rawBMP.block != nullptr);

	BMPHeader* header{ rcast<BMPHeader*>(rawBMP.block) };
	uint8_t* pPixels{ rcast<uint8_t*>(rawBMP.block) + header->pxOffset };

	ASSERT(header->pxWidth > 0);
	ASSERT(header->pxHeight > 0);
	ASSERT(header->bitsPerPixel == 32 || header->bitsPerPixel == 24);
	ASSERT(header->compressionMethod == 3 || header->compressionMethod == 0);

	size_t absWidth{ pstd::abs(header->pxWidth) };
	size_t absHeight{ pstd::abs(header->pxHeight) };

	size_t nPixels{ absHeight * absWidth };
	uint32_t* pixelArray{ pstd::alloc<uint32_t>(pArena, nPixels) };

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

	pstd::FirstSetBit redShift{ pstd::bitscanForward(redMask) };
	pstd::FirstSetBit greenShift{ pstd::bitscanForward(greenMask) };
	pstd::FirstSetBit blueShift{ pstd::bitscanForward(blueMask) };
	pstd::FirstSetBit alphaShift{ pstd::bitscanForward(alphaMask) };

	ASSERT(redShift.found);
	ASSERT(greenShift.found);
	ASSERT(blueShift.found);
	ASSERT(alphaShift.found);

	// BMP pixels are aligned to 4byte boundarys
	size_t stride{ absWidth * (header->bitsPerPixel / 8) };
	bool imageFlipped{ header->pxHeight < 0 };
	stride = (stride + 3) & ~3;
	for (size_t dstY{}; dstY < absHeight; dstY++) {
		size_t srcY{ dstY };
		if (imageFlipped) {
			srcY = absHeight - dstY - 1;
		}

		for (int x{}; x < absWidth; x++) {
			size_t colorByteIndex{ (srcY * stride) +
								   (x * (header->bitsPerPixel / 8)) };
			uint32_t color{ *rcast<uint32_t*>(pPixels + colorByteIndex) };
			if (header->compressionMethod == 0) {
				color = color | 0xFF << 24;
			}

			pixelArray[(dstY * absWidth) + x] =
				(((color >> redShift.shift) & 0xFF) << 0) |
				(((color >> greenShift.shift) & 0xFF) << 8) |
				(((color >> blueShift.shift) & 0xFF) << 16) |
				(((color >> alphaShift.shift) & 0xFF) << 24);
		}
	}

	return Engine::TextureData{
		.pPixels = pixelArray,
		.width = absWidth,
		.height = absHeight,
	};
}

Engine::MeshData Engine::loadOBJ(
	pstd::Arena* pArena, pstd::Arena scratchArena, const pstd::String path
) {
	pstd::String lines{ pstd::createString(pstd::readFile(&scratchArena, path)
	) };

	OBJ obj{ parseOBJ(&scratchArena, *pArena, lines) };
	uint32_t indexCount{
		ncast<uint32_t>(max(obj.positionIndices.count, obj.uvIndices.count))
	};
	uint32_t vertexCount{ indexCount };

	auto* pPositions{ pstd::alloc<pstd::Vec3>(pArena, indexCount) };
	auto* pIndices{ pstd::alloc<uint32_t>(pArena, indexCount) };
	auto* pUVs{ pstd::alloc<pstd::Vec2>(pArena, indexCount) };

	for (uint32_t i{}; i < indexCount; i++) {
		pPositions[i] = obj.uniquePositions[obj.positionIndices[i]];
	}
	for (uint32_t i{}; i < indexCount; i++) {
		pUVs[i] = obj.uniqueUVs[obj.uvIndices[i]];
	}

	for (uint32_t i{}; i < indexCount; i++) {
		pIndices[i] = i;
	}

	return Engine::MeshData{ .vertexCount = vertexCount,
							 .indexCount = indexCount,
							 .pPositions = pPositions,
							 .pIndices = pIndices,
							 .pUVs = pUVs };
}

namespace {

	OBJMetadata parseOBJMetadata(pstd::String objString) {
		uint32_t positionCount{};
		uint32_t uvCount{};
		uint32_t vertexCount{};
		while (objString.size > 0) {
			pstd::String line{ pstd::readLine(&objString) };
			pstd::String identifier{ pstd::readToken(&line) };

			if (identifier == "v") {
				positionCount++;
			} else if (identifier == "vt") {
				uvCount++;
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
							.uvCount = uvCount,
							.vertexCount = vertexCount };
	}

	void parseFace(
		const pstd::Span<pstd::String>& contents,
		pstd::Arena scratchArena,
		pstd::Array<uint32_t>* pPositionIndices,
		pstd::Array<uint32_t>* pUVIndices
	) {
		ASSERT(contents.count <= 4);

		uint32_t positionIndices[4];
		uint32_t uvIndices[4];

		for (size_t i{}; i < contents.count; i++) {
			pstd::Array<pstd::String> face{
				pstd::split(&scratchArena, contents[i], 3, "/")
			};

			ASSERT(face.count == 3);

			positionIndices[i] = pstd::parse<uint32_t>(face[0]);
			uvIndices[i] = pstd::parse<uint32_t>(face[1]);
		}

		if (contents.count == 3) {
			// -1 because obj is 1 indexed, reverese index to do cw -> ccw
			pstd::pushBack(pPositionIndices, positionIndices[2] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[1] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[0] - 1);

			pstd::pushBack(pUVIndices, uvIndices[2] - 1);
			pstd::pushBack(pUVIndices, uvIndices[1] - 1);
			pstd::pushBack(pUVIndices, uvIndices[0] - 1);
		} else {
			pstd::pushBack(pPositionIndices, positionIndices[2] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[1] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[0] - 1);

			pstd::pushBack(pPositionIndices, positionIndices[3] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[2] - 1);
			pstd::pushBack(pPositionIndices, positionIndices[0] - 1);

			pstd::pushBack(pUVIndices, uvIndices[2] - 1);
			pstd::pushBack(pUVIndices, uvIndices[1] - 1);
			pstd::pushBack(pUVIndices, uvIndices[0] - 1);

			pstd::pushBack(pUVIndices, uvIndices[3] - 1);
			pstd::pushBack(pUVIndices, uvIndices[2] - 1);
			pstd::pushBack(pUVIndices, uvIndices[0] - 1);
		}
	}

	OBJ parseOBJ(
		pstd::Arena* pArena, pstd::Arena scratchArena, pstd::String lines
	) {
		OBJMetadata meta{ parseOBJMetadata(lines) };

		auto uniquePositions{
			pstd::createArray<pstd::Vec3>(pArena, meta.positionCount, 0)
		};

		auto uniqueUVs{
			pstd::createArray<pstd::Vec2>(pArena, meta.uvCount, 0)
		};

		auto positionIndices{
			pstd::createArray<uint32_t>(pArena, meta.vertexCount, 0)
		};
		auto uvIndices{
			pstd::createArray<uint32_t>(pArena, meta.vertexCount, 0)
		};

		while (lines.size > 0) {
			pstd::String line{ pstd::readLine(&lines) };
			pstd::String identifier{ pstd::readToken(&line) };

			pstd::Array<pstd::String> contents{
				pstd::split(&scratchArena, line, 4)
			};

			if (identifier == "f") {
				parseFace(contents, scratchArena, &positionIndices, &uvIndices);
			} else if (identifier == "v") {
				pstd::Vec3 position{ pstd::parse<float>(contents[0]),
									 pstd::parse<float>(contents[1]),
									 pstd::parse<float>(contents[2]) };
				pstd::pushBack(&uniquePositions, position);
			} else if (identifier == "vt") {
				pstd::Vec2 uv{ pstd::parse<float>(contents[0]),
							   pstd::parse<float>(contents[1]) };
				pstd::pushBack(&uniqueUVs, uv);
			}
		}

		return OBJ{ .uniquePositions = uniquePositions,
					.uniqueUVs = uniqueUVs,
					.positionIndices = positionIndices,
					.uvIndices = uvIndices };
	}

}  // namespace
