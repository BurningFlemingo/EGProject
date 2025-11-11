#include "AssetLoader.h"
#include "STD/PFileIO.h"
#include "STD/PAlgorithm.h"

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
}  // namespace

pstd::BMP pstd::loadBMP(pstd::Arena* pArena, const char* path) {
	pstd::Allocation rawBMP{ pstd::readFile(pArena, path) };
	if (rawBMP.size == 0 || rawBMP.block == nullptr) {
		return {};
	}

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

pstd::OBJ pstd::loadOBJ(
	pstd::Arena* pArena, pstd::Arena scratchArena, const char* path
) {
	pstd::String objString{
		pstd::createString(pstd::readFile(&scratchArena, path))
	};

	while (objString.size > 0) {
		pstd::String line{ pstd::readLine(&objString) };

		pstd::Array<pstd::String> elements{
			splitLine(&scratchArena, line, ' ')
		};

		if (pstd::stringsMatch(elements[0], pstd::createString("n"))) {}
	}

	return {};
}
