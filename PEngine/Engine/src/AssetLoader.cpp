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
}  // namespace

pstd::BMP pstd::loadBMP(pstd::Arena* pArena, const pstd::String path) {
	pstd::Allocation rawBMP{ pstd::readFile(pArena, path) };

	ASSERT(rawBMP.size != 0);
	ASSERT(rawBMP.block != nullptr);

	BMPHeader* header{ rcast<BMPHeader*>(rawBMP.block) };
	uint32_t* pPixels{ rcast<uint32_t*>(rawBMP.block) + header->pxOffset };

	ASSERT(header->pxWidth > 0);
	ASSERT(header->pxHeight > 0);
	ASSERT(header->compressionMethod == 3);
	ASSERT(header->bitsPerPixel == 32);

	size_t absWidth{ pstd::abs(header->pxWidth) };
	size_t absHeight{ pstd::abs(header->pxHeight) };

	pstd::BMP bmp{
		.pPixels = pPixels,
		.width = absWidth,
		.height = absHeight,
	};

	uint32_t redMask{ header->redMask };
	uint32_t greenMask{ header->greenMask };
	uint32_t blueMask{ header->blueMask };
	uint32_t alphaMask{ ~(redMask | greenMask | blueMask) };

	pstd::FirstSetBit redShift{ pstd::bitscanForward(redMask) };
	pstd::FirstSetBit greenShift{ pstd::bitscanForward(greenMask) };
	pstd::FirstSetBit blueShift{ pstd::bitscanForward(blueMask) };
	pstd::FirstSetBit alphaShift{ pstd::bitscanForward(alphaMask) };

	ASSERT(redShift.found);
	ASSERT(greenShift.found);
	ASSERT(blueShift.found);
	ASSERT(alphaShift.found);

	uint32_t* pPixel{ pPixels };
	for (int y{}; y < absHeight; y++) {
		for (int x{}; x < absWidth; x++) {
			uint32_t color{ *pPixel };
			*pPixel = (((color >> redShift.shift) & 0xFF) << 0) |
				(((color >> greenShift.shift) & 0xFF) << 8) |
				(((color >> blueShift.shift) & 0xFF) << 16) |
				(((color >> alphaShift.shift) & 0xFF) << 24);
			pPixel++;
		}
	}

	return bmp;
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

pstd::OBJ pstd::loadOBJ(
	pstd::Arena* pArena, pstd::Arena scratchArena, const pstd::String path
) {
	pstd::String ogObjString{
		pstd::createString(pstd::readFile(&scratchArena, path))
	};
	pstd::String objString{ ogObjString };

	uint32_t nPositions{};
	uint32_t nIndices{};
	while (objString.size > 0) {
		pstd::String line{ pstd::readLine(&objString) };

		pstd::Array<pstd::String> elements{
			pstd::splitLine(&scratchArena, line, pstd::String{ " " })
		};

		pstd::String identifier{ elements[0] };

		if (pstd::stringsMatch(identifier, pstd::createString("v"))) {
			nPositions++;
		} else if (pstd::stringsMatch(identifier, pstd::createString("f"))) {
			// TODO: make this more modular
			nIndices += 6;
		}
	}

	auto uniquePositions{
		pstd::createArray<pstd::Vec4>(pArena, nPositions, 0)
	};
	auto indices{ pstd::createArray<uint32_t>(pArena, nIndices, 0) };

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
				pstd::Array<pstd::String> face{
					pstd::splitLine(&scratchArena, stringIndicies[i], "/")
				};
				uint32_t index{ pstd::parse<uint32_t>(face[0]) -
								1 };  // -1 because OBJ indices are 1 indexed

				pstd::pushBack(&indices, index);
			}
		}
		if (pstd::stringsMatch(items[0], "v")) {
			pstd::Vec4 positions{ pstd::parse<float>(items[1]),
								  pstd::parse<float>(items[2]),
								  pstd::parse<float>(items[3]),
								  1.f };
			pstd::pushBack(&uniquePositions, positions);
		}
	}

	LOG_INFO("nPositions: %u, nIndices: %u \nIndices:", nPositions, nIndices);
	for (size_t i{}; i < indices.count; i++) {
		LOG_INFO("%u\n", indices[i]);
	}
	LOG_INFO("\nPositions:");

	for (size_t i{}; i < uniquePositions.count; i++) {
		pstd::Vec4 pos{ uniquePositions[i] };
		LOG_INFO("(%f, %f, %f)\n", pos.x, pos.y, pos.z);
	}
	LOG_INFO("\n");

	return OBJ{ .uniquePositions = uniquePositions, .indices = indices };
}
