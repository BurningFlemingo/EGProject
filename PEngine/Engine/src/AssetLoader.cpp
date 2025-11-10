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

	ASSERT(header->compressionMethod == 3);
	ASSERT(header->bitsPerPixel == 32);

	pstd::BMP bmp{
		.pPixels = pPixels,
		// .width = pstd::abs(header->pxWidth),
		// .height = pstd::abs(header->pxHeight),
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

	uint32_t* pPixel{};
}

// BMP DEBUGLoadBPM(const char* filePath) {
// 	Platform::DEBUGReadFileResult file{ readEntireFile(thread, filePath) };
// 	if (!file.contents || file.size == 0) {
// 		return {};
// 	}
// 	BitmapHeader* bmpHeader{ reinterpret_cast<BitmapHeader*>(file.contents) };
// 	uint32_t* bmpPixelData{ reinterpret_cast<uint32_t*>(
// 		static_cast<uint8_t*>(file.contents) + bmpHeader->bitmapOffset
// 	) };
//
// 	ASSERT(bmpHeader->compressionMethod == 3);
//
// 	BMP bmp{};
// 	bmp.file = file.contents;
// 	bmp.height = bmpHeader->height;
// 	bmp.width = bmpHeader->width;
// 	bmp.pixels = bmpPixelData;
//
// 	uint32_t redMask{ bmpHeader->redMask };
// 	uint32_t greenMask{ bmpHeader->greenMask };
// 	uint32_t blueMask{ bmpHeader->blueMask };
// 	uint32_t alphaMask{ ~(redMask | greenMask | blueMask) };
//
// 	FirstSetBit redShift{ findFirstSetBit(redMask) };
// 	FirstSetBit greenShift{ findFirstSetBit(greenMask) };
// 	FirstSetBit blueShift{ findFirstSetBit(blueMask) };
// 	FirstSetBit alphaShift{ findFirstSetBit(alphaMask) };
//
// 	ASSERT(redShift.found);
// 	ASSERT(greenShift.found);
// 	ASSERT(blueShift.found);
// 	ASSERT(alphaShift.found);
//
// 	uint32_t* pixel{ bmpPixelData };
// 	for (int y{}; y < bmp.height; y++) {
// 		for (int x{}; x < bmp.width; x++) {
// 			uint32_t color{ *pixel };
// 			*pixel =
// 				((((color >> alphaShift.shift) & 0xFF) << 24) |
// 				 (((color >> redShift.shift) & 0xFF) << 16) |
// 				 (((color >> greenShift.shift) & 0xFF) << 8) |
// 				 (((color >> blueShift.shift) & 0xFF) << 0));
// 			pixel++;
// 		}
// 	}
// 	return bmp;
// }
