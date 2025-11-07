#include "AssetLoader.h"

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
