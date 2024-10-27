#include "PArena.h"
#include "public/PString.h"
#include "public/PTypes.h"
#include "public/PAlgorithm.h"
#include "public/PArray.h"
#include "public/PMath.h"

using namespace pstd;

namespace {
	String uint32_tToString(pstd::FixedArena* buffer, uint32_t number);
	String int32_tToString(pstd::FixedArena* buffer, int32_t number);
	String floatToString(
		pstd::FixedArena* buffer, float number, uint32_t precision = 5
	);

	String letterToString(pstd::FixedArena* buffer, char letter);
}  // namespace

uint32_t pstd::getCStringLength(const char* cString) {
	const uint32_t maxStringSize{
		1024
	};	// to avoid an infinite loop for ill-formed cstrings

	uint32_t stringSize{};
	while (stringSize < 1024 && *cString != '\0') {
		stringSize++;
		cString++;
	}

	return stringSize;
}

pstd::String pstd::createString(const char* cString) {
	uint32_t stringSize{ getCStringLength(cString) };
	pstd::String string{ .buffer = cString, .size = stringSize };
	return string;
}

template<typename T>
String pstd::formatString(pstd::FixedArena* buffer, String format, T val) {
	void* const initialBufferAddress{ pstd::getNextAllocAddress<char>(*buffer
	) };
	const size_t initialBufferCountAvaliable{
		pstd::getAvaliableCount<char>(*buffer)
	};

	size_t searchSize{ min(initialBufferCountAvaliable, format.size) };

	char previousLetter{};
	for (size_t i{}; i < searchSize; i++) {
		char currentLetter{ format.buffer[i] };

		if (previousLetter == '%') {
			switch (currentLetter) {
				case 'i': {
					int32_tToString(buffer, (int32_t)val);
				} break;
				case 'u': {
					uint32_tToString(buffer, (uint32_t)val);
				} break;
				case 'f': {
					floatToString(buffer, (float)val);
				} break;
				default:
					letterToString(buffer, '%');
					letterToString(buffer, currentLetter);
					break;
			}
		} else if (currentLetter != '%') {
			letterToString(buffer, currentLetter);
		}

		previousLetter = currentLetter;
	}
	size_t finalBufferCountAvaliable{ pstd::getAvaliableCount<char>(*buffer) };
	size_t size{ initialBufferCountAvaliable - finalBufferCountAvaliable };
	String res{ .buffer = (const char*)initialBufferAddress, .size = size };
	return res;
}

namespace {
	String floatToString(
		pstd::FixedArena* buffer, float number, uint32_t precision
	) {
		ASSERT(precision < 128);  // to avoid a loop that blows out the buffer
		ASSERT(buffer);

		void* const initialBufferAddress{
			pstd::getNextAllocAddress<char>(*buffer)
		};
		const size_t initialBufferCountAvaliable{
			pstd::getAvaliableCount<char>(*buffer)
		};

		int32_t integerPart{ (int32_t)number };
		String integerPartString{ int32_tToString(buffer, integerPart) };
		uint32_t decimalPartSize{
			1 + precision
		};	// the plus one is for the decimal itself

		decimalPartSize = min(decimalPartSize, initialBufferCountAvaliable);

		if (decimalPartSize > 0) {
			FixedArray<char> decimalArray{
				.allocation = pstd::bufferAlloc<char>(buffer, 1)
			};
			pstd::indexWrite(&decimalArray, 0, '.');
		}

		FixedArray<char> decimalPartArray{
			.allocation = pstd::bufferAlloc<char>(buffer, decimalPartSize)
		};

		float decimalPart{ pstd::absf(number - integerPart) };
		for (uint32_t i{}; i < decimalPartSize; i++) {
			decimalPart *= 10.f;
			uint32_t digit{ (uint32_t)decimalPart % 10 };
			char digitLetter{ (char)('0' + digit) };
			pstd::indexWrite(&decimalPartArray, i, digitLetter);
		}

		size_t finalBufferCountAvaliable{ pstd::getAvaliableCount<char>(*buffer
		) };
		size_t size{ initialBufferCountAvaliable - finalBufferCountAvaliable };
		String res{ .buffer = (const char*)initialBufferAddress, .size = size };
		return res;
	}
	String int32_tToString(pstd::FixedArena* buffer, int32_t number) {
		ASSERT(buffer);

		void* const initialBufferAddress{
			pstd::getNextAllocAddress<char>(*buffer)
		};
		const size_t initialBufferCountAvaliable{
			pstd::getAvaliableCount<char>(*buffer)
		};

		String string{};
		if (number >= 0) {
			string = uint32_tToString(buffer, (uint32_t)number);
			return string;
		}

		pstd::FixedArray<char> signLetter{
			.allocation = pstd::bufferAlloc<char>(buffer, 1)
		};
		pstd::indexWrite(&signLetter, 0, '-');

		// this avoids overflow since |INT_MIN| = |INT_MAX| + 1
		uint32_t positiveNumber{ (uint32_t)(-(number + 1) + 1) };
		String positiveString{ uint32_tToString(buffer, positiveNumber) };

		size_t finalBufferCountAvaliable{ pstd::getAvaliableCount<char>(*buffer
		) };
		size_t size{ initialBufferCountAvaliable - finalBufferCountAvaliable };
		String res{ .buffer = (const char*)initialBufferAddress, .size = size };
		return res;
	}

	String uint32_tToString(pstd::FixedArena* buffer, uint32_t number) {
		ASSERT(buffer);

		void* const initialBufferAddress{
			pstd::getNextAllocAddress<char>(*buffer)
		};
		const size_t initialBufferCountAvaliable{
			pstd::getAvaliableCount<char>(*buffer)
		};

		uint32_t count{ 1 };
		size_t bufferCountAvaliable{ pstd::getAvaliableCount<char>(*buffer) };

		{
			uint32_t numberCopy{ number };
			while (numberCopy > 9 && count < bufferCountAvaliable) {
				numberCopy /= 10;
				count++;
			}
		}

		pstd::FixedArray<char> letterArray{
			.allocation = pstd::bufferAlloc<char>(buffer, count),
		};

		for (uint32_t i{}; i < count; i++) {
			uint32_t reverseIndex{ (count - 1) - i };
			uint32_t digit{ number % 10 };
			char digitLetter{ (char)((uint32_t)'0' + digit) };
			pstd::indexWrite(&letterArray, reverseIndex, digitLetter);
			number /= 10;
		}

		size_t finalBufferCountAvaliable{ pstd::getAvaliableCount<char>(*buffer
		) };
		size_t size{ initialBufferCountAvaliable - finalBufferCountAvaliable };
		String res{ .buffer = (const char*)initialBufferAddress, .size = size };
		return res;
	}

	String letterToString(pstd::FixedArena* buffer, char letter) {
		String res{};
		if (pstd::getAvaliableCount<char>(*buffer) == 0) {
			return res;
		}
		pstd::FixedArray<char> letterArray{
			pstd::bufferAlloc<char>(buffer, 1)
		};
		pstd::indexWrite(&letterArray, 0, letter);
		res = { .buffer = (const char*)letterArray.allocation.block,
				.size = 1 };
		return res;
	}
}  // namespace

template String
	pstd::formatString(pstd::FixedArena* buffer, String format, uint32_t val);
template String
	pstd::formatString(pstd::FixedArena* buffer, String format, int32_t val);
template String
	pstd::formatString(pstd::FixedArena* buffer, String format, float val);
