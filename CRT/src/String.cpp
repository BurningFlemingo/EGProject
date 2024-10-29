#include "PArena.h"
#include "public/PString.h"
#include "public/PTypes.h"
#include "public/PAlgorithm.h"
#include "public/PArray.h"
#include "public/PMath.h"
#include "public/PMemory.h"

using namespace pstd;

namespace {
	String uint32_tToString(pstd::FixedArena* buffer, uint32_t number);
	String int32_tToString(pstd::FixedArena* buffer, int32_t number);
	String floatToString(
		pstd::FixedArena* buffer, float number, uint32_t precision = 5
	);

	String letterToString(pstd::FixedArena* buffer, char letter);
}  // namespace

String pstd::makeNullTerminated(FixedArena* buffer, const String& string) {
	ASSERT(string.buffer);
	String nullTerminatedString{};
	if (string.size == 0) {
		return string;
	}

	size_t lastLetterIndex{ string.size - 1 };
	if (string.buffer[lastLetterIndex] == '\0') {
		return string;
	}

	void* const initialBufferAddress{ pstd::getNextAllocAddress<char>(*buffer
	) };
	const size_t initialBufferCountAvaliable{
		pstd::getAvaliableCount<char>(*buffer)
	};

	size_t lettersToCopy{ min(initialBufferCountAvaliable, string.size) };
	Allocation stringAllocation{
		pstd::arenaAlloc<char>(buffer, lettersToCopy)
	};
	pstd::memCpy(stringAllocation.block, string.buffer, lettersToCopy);
	letterToString(buffer, '\0');

	size_t finalBufferCountAvaliable{ pstd::getAvaliableCount<char>(*buffer) };
	size_t size{ initialBufferCountAvaliable - finalBufferCountAvaliable };
	String res{ .buffer = (const char*)initialBufferAddress, .size = size };
	return res;
}

bool pstd::stringsMatch(const String& a, const String& b) {
	ASSERT(a.buffer);
	ASSERT(b.buffer);

	bool res{ true };

	if (a.size != b.size) {
		res = false;
		return res;
	}
	res = memcmp(a.buffer, b.buffer, a.size) == 0;
	return res;
}

template<typename T>
String
	pstd::formatString(pstd::FixedArena* buffer, const String& format, T val) {
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

String pstd::getFileName(const String& string) {
	size_t fileNameSize{};
	for (uint32_t i{}; i < string.size; i++) {
		uint32_t reverseI{ (uint32_t)(string.size) - 1 - i };

		char letter{ string.buffer[reverseI] };
		if (letter == '\\' || letter == '/') {
			break;
		}
		fileNameSize++;
	}
	size_t pathSize{ string.size - fileNameSize };
	const char* address{ string.buffer + pathSize };
	String fileName{ .buffer = address, .size = fileNameSize };
	return fileName;
}

String pstd::getFileName(const char* cString) {
	String res{ getFileName(pstd::createString(cString)) };
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

		if (number < 0) {
			letterToString(buffer, '-');
			number *= -1.f;
		}

		uint32_t wholePart{ (uint32_t)number };
		String wholePartString{ uint32_tToString(buffer, wholePart) };

		letterToString(buffer, '.');

		uint32_t decimalPartSize{
			min(precision, (uint32_t)initialBufferCountAvaliable)
		};
		FixedArray<char> decimalPartArray{
			.allocation = pstd::arenaAlloc<char>(buffer, decimalPartSize)
		};

		float decimalPart{ pstd::absf(number - wholePart) };
		for (uint32_t i{}; i < decimalPartSize; i++) {
			decimalPart *= 10.f;
			uint32_t digit{ (uint32_t)decimalPart % 10 };
			char digitLetter{ (char)('0' + digit) };
			decimalPartArray[i] = digitLetter;
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

		letterToString(buffer, '-');

		// this avoids overflow since |INT_MIN| = |INT_MAX| + 1
		uint32_t positiveNumber{ (uint32_t)(-(number + 1)) + 1 };
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
			.allocation = pstd::arenaAlloc<char>(buffer, count),
		};

		for (uint32_t i{}; i < count; i++) {
			uint32_t reverseIndex{ (count - 1) - i };
			uint32_t digit{ number % 10 };
			char digitLetter{ (char)((uint32_t)'0' + digit) };
			letterArray[reverseIndex] = digitLetter;
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
			.allocation = pstd::arenaAlloc<char>(buffer, 1)
		};
		letterArray[0] = letter;
		res = { .buffer = (const char*)letterArray.allocation.block,
				.size = 1 };
		return res;
	}
}  // namespace

template String pstd::formatString(
	pstd::FixedArena* buffer, const String& format, uint32_t val
);
template String pstd::formatString(
	pstd::FixedArena* buffer, const String& format, int32_t val
);
template String pstd::formatString(
	pstd::FixedArena* buffer, const String& format, float val
);
