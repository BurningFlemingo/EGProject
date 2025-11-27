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

	pstd::Array<uint32_t>
		triangulate(pstd::Arena* pArena, const pstd::Array<uint32_t>& indices);

	struct OBJ {
		pstd::Array<pstd::Vec3> uniquePositions;
		pstd::Array<pstd::Vec2> uniqueUVs;

		pstd::Array<uint32_t> positionIndices;
		pstd::Array<uint32_t> uvIndices;
	};
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
	stride = (stride + 3) & ~3;
	for (int y{}; y < absHeight; y++) {
		for (int x{}; x < absWidth; x++) {
			size_t colorByteIndex{ (y * stride) +
								   (x * (header->bitsPerPixel / 8)) };
			uint32_t color{ *rcast<uint32_t*>(pPixels + colorByteIndex) };
			if (header->compressionMethod == 0) {
				color = color | 0xFF << 24;
			}

			pixelArray[(y * absWidth) + x] =
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

pstd::Array<pstd::String>
	triangulate(pstd::Arena* pArena, const pstd::Array<pstd::String>& indices) {
	if (indices.count == 3) {
		return indices;
	}
	ASSERT(indices.count == 4, "dont support triangulating ngons n > 4");

	auto newIndices{ pstd::createArray<pstd::String>(pArena, 6) };

	newIndices[0] = indices[0];
	newIndices[1] = indices[1];
	newIndices[2] = indices[2];

	newIndices[3] = indices[0];
	newIndices[4] = indices[2];
	newIndices[5] = indices[3];

	return newIndices;
}

Engine::MeshData Engine::loadOBJ(
	pstd::Arena* pArena, pstd::Arena scratchArena, const pstd::String path
) {
	pstd::String ogObjString{
		pstd::createString(pstd::readFile(&scratchArena, path))
	};
	pstd::String objString{ ogObjString };

	uint32_t nPositions{};
	uint32_t nIndices{};
	uint32_t nUVs{};
	while (objString.size > 0) {
		pstd::String line{ pstd::readLine(&objString) };

		pstd::Array<pstd::String> elements{
			pstd::splitLine(&scratchArena, line, pstd::String{ " " })
		};

		pstd::String identifier{ elements[0] };

		if (pstd::stringsMatch(identifier, pstd::createString("v"))) {
			nPositions++;
		} else if (pstd::stringsMatch(identifier, pstd::createString("vt"))) {
			nUVs++;
		} else if (pstd::stringsMatch(identifier, pstd::createString("f"))) {
			size_t ngonIndexCount{
				pstd::splitLine(&scratchArena, line, " ").count
			};
			if (ngonIndexCount == 4) {
				nIndices += 6;	// because of triangulation;
			} else {
				nIndices += 3;
			}

			ASSERT(
				ngonIndexCount == 3 || ngonIndexCount == 4,
				"obj has ngons that arent tris or quads"
			);
		}
	}

	auto uniquePositions{
		pstd::createArray<pstd::Vec3>(pArena, nPositions, 0)
	};
	auto uniqueUVs{ pstd::createArray<pstd::Vec2>(pArena, nUVs, 0) };

	auto positionIndices{ pstd::createArray<uint32_t>(pArena, nIndices, 0) };
	auto uvIndices{ pstd::createArray<uint32_t>(pArena, nIndices, 0) };

	objString = ogObjString;
	while (objString.size > 0) {
		pstd::String line{ pstd::readLine(&objString) };

		pstd::Array<pstd::String> items{
			pstd::splitLine(&scratchArena, line, " ")
		};

		if (pstd::stringsMatch(items[0], "f")) {
			pstd::Array<pstd::String> stringIndicies{
				triangulate(&scratchArena, pstd::makeSliced(items, 1))
			};
			for (size_t i{}; i < stringIndicies.count; i++) {
				size_t reverseI{ (stringIndicies.count - 1) -
								 i };  // cw data -> ccw data

				pstd::Array<pstd::String> face{ pstd::splitLine(
					&scratchArena, stringIndicies[reverseI], "/"
				) };

				// -1 because OBJ indices are 1 indexed
				uint32_t positionIndex{ pstd::parse<uint32_t>(face[0]) - 1 };
				uint32_t uvIndex{ pstd::parse<uint32_t>(face[1]) - 1 };

				pstd::pushBack(&positionIndices, positionIndex);
				pstd::pushBack(&uvIndices, uvIndex);
			}
		} else if (pstd::stringsMatch(items[0], "v")) {
			pstd::Vec3 positions{ pstd::parse<float>(items[1]),
								  pstd::parse<float>(items[2]),
								  pstd::parse<float>(items[3]) };
			pstd::pushBack(&uniquePositions, positions);
		} else if (pstd::stringsMatch(items[0], "vt")) {
			pstd::Vec2 uvs{ pstd::parse<float>(items[1]),
							pstd::parse<float>(items[2]) };
			pstd::pushBack(&uniqueUVs, uvs);
		}
	}

	// LOG_INFO("nPositions: %u, nIndices: %u \nIndices:", nPositions,
	// nIndices); for (size_t i{}; i < indices.count; i++) { 	LOG_INFO("%u\n",
	// indices[i]);
	// }
	// LOG_INFO("\nPositions:");

	// for (size_t i{}; i < uniquePositions.count; i++) {
	// 	pstd::Vec3 pos{ uniquePositions[i] };
	// 	LOG_INFO("(%f, %f, %f)\n", pos.x, pos.y, pos.z);
	// }
	// LOG_INFO("\n");

	return MeshData{ .uniquePositions = uniquePositions,
					 .uniqueUVs = uniqueUVs,
					 .positionIndices = positionIndices,
					 .uvIndices = uvIndices };
}
