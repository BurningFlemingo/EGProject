#include "PArena.h"
#include "public/PString.h"
#include "public/PTypes.h"
#include "public/PAlgorithm.h"
#include "public/PArray.h"
#include "public/PMath.h"
#include "public/PMemory.h"

using namespace pstd;

namespace {
	size_t pushUInt32AsString(
		pstd::FixedArena* buffer, uint32_t number
	);	// returns size of string pushed
	size_t pushInt32AsString(
		pstd::FixedArena* buffer, int32_t number
	);	// returns size of string pushed
	size_t pushFloatAsString(
		pstd::FixedArena* buffer, float number, uint32_t precision = 5
	);	// returns size of string pushed

	size_t pushString(
		pstd::FixedArena* buffer, const String& string
	);	// returns size of string pushed

	size_t pushStringUntilControlCharacter(
		pstd::FixedArena* buffer,
		const String& format,
		size_t* outFormatCharactersProccessed,
		char* outControlCharacter
	);
	// returns size of string pushed

	size_t pushLetter(pstd::FixedArena* buffer, char letter);
}  // namespace

String pstd::makeNullTerminated(FixedArena* buffer, String string) {
	ASSERT(string.buffer);
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
	pushLetter(buffer, '\0');

	size_t finalBufferCountAvaliable{ pstd::getAvaliableCount<char>(*buffer) };
	size_t size{ initialBufferCountAvaliable - finalBufferCountAvaliable };

	string =
		String{ .buffer = (const char*)initialBufferAddress, .size = size };
	return string;
}

bool pstd::stringsMatch(const String& a, const String& b) {
	ASSERT(a.buffer);
	ASSERT(b.buffer);

	if (a.size != b.size) {
		return false;
	}
	return memcmp(a.buffer, b.buffer, a.size) == 0;
}

String pstd::concat(pstd::FixedArena* buffer, String a, String b) {
	void* const stringAddress{ pstd::getNextAllocAddress<char>(*buffer) };

	size_t stringSize{ pushString(buffer, a) };
	stringSize += pushString(buffer, b);

	String res{ .buffer = (const char*)stringAddress, .size = stringSize };
	return res;
}

String pstd::formatString(pstd::FixedArena* buffer, const String& format) {
	ASSERT(buffer);

	const void* stringAddress{ pstd::getNextAllocAddress<char>(*buffer) };
	size_t stringSize{ pushString(buffer, format) };

	String res{ .buffer = (const char*)stringAddress, .size = stringSize };
	return res;
}

template<typename T>
String
	pstd::formatString(pstd::FixedArena* buffer, const String& format, T val) {
	ASSERT(buffer);
	const void* stringAddress{ pstd::getNextAllocAddress<char>(*buffer) };
	const size_t initialBufferCountAvaliable{
		pstd::getAvaliableCount<char>(*buffer)
	};

	char controlCharacter{};
	size_t formatCharactersProccessed{};
	size_t stringSize{ pushStringUntilControlCharacter(
		buffer, format, &formatCharactersProccessed, &controlCharacter
	) };
	switch (controlCharacter) {
		case 'i': {
			stringSize += pushInt32AsString(buffer, (int32_t)val);
		} break;
		case 'u': {
			stringSize += pushUInt32AsString(buffer, (uint32_t)val);
		} break;
		case 'f': {
			stringSize += pushFloatAsString(buffer, (float)val);
		} break;
		default:
			break;
	}
	auto stringFormatSizeDifference{ (size_t
	)pstd::abs((int)format.size - (int)stringSize) };

	if (formatCharactersProccessed < format.size) {
		String restOfFormat{ .buffer =
								 format.buffer + formatCharactersProccessed,
							 .size = format.size - formatCharactersProccessed };

		stringSize += pushString(buffer, restOfFormat);
	}

	String res{ .buffer = (const char*)stringAddress, .size = stringSize };
	return res;
}

template<>
String pstd::formatString(
	pstd::FixedArena* buffer, const String& format, pstd::String val
) {
	ASSERT(buffer);
	const void* stringAddress{ pstd::getNextAllocAddress<char>(*buffer) };

	char controlCharacter{};
	size_t formatCharactersProccessed{};
	size_t stringSize{ pushStringUntilControlCharacter(
		buffer, format, &formatCharactersProccessed, &controlCharacter
	) };
	if (controlCharacter == 'm') {
		stringSize += pushString(buffer, val);
	}

	if (formatCharactersProccessed < format.size) {
		String restOfFormat{ .buffer =
								 format.buffer + formatCharactersProccessed,
							 .size = format.size - formatCharactersProccessed };

		stringSize += pushString(buffer, restOfFormat);
	}

	String res{ .buffer = (const char*)stringAddress, .size = stringSize };
	return res;
}

template<>
String pstd::formatString(
	pstd::FixedArena* buffer, const String& format, const char* val
) {
	return formatString(buffer, format, createString(val));
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
	String res{ .buffer = address, .size = fileNameSize };
	return res;
}

String pstd::getFileName(const char* cString) {
	return getFileName(pstd::createString(cString));
}

namespace {
	size_t pushFloatAsString(
		pstd::FixedArena* buffer, float number, uint32_t precision
	) {
		ASSERT(precision < 128);  // to avoid a loop that blows out the buffer
		ASSERT(buffer);

		size_t stringSize{};

		if (number < 0) {
			stringSize += pushLetter(buffer, '-');
			number *= -1.f;
		}

		uint32_t wholePart{ (uint32_t)number };
		stringSize += pushUInt32AsString(buffer, wholePart);

		stringSize += pushLetter(buffer, '.');

		size_t factor{ pstd::pow<size_t>(10, precision) };
		auto decimalPart{ (uint32_t)((pstd::absf(number - wholePart) * factor) +
									 0.5f) };
		stringSize += pushUInt32AsString(buffer, decimalPart);
		return stringSize;
	}

	size_t pushInt32AsString(pstd::FixedArena* buffer, int32_t number) {
		ASSERT(buffer);

		size_t stringSize{};
		if (number >= 0) {
			return pushUInt32AsString(buffer, (uint32_t)number);
		}

		stringSize += pushLetter(buffer, '-');

		// this avoids overflow since |INT_MIN| = |INT_MAX| + 1
		uint32_t positiveNumber{ (uint32_t)(-(number + 1)) + 1 };
		stringSize += pushUInt32AsString(buffer, positiveNumber);
		return stringSize;
	}

	size_t pushUInt32AsString(pstd::FixedArena* buffer, uint32_t number) {
		ASSERT(buffer);

		uint32_t count{ 1 };
		{
			size_t bufferCountAvaliable{ pstd::getAvaliableCount<char>(*buffer
			) };
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
		return letterArray.allocation.size;
	}

	size_t pushLetter(pstd::FixedArena* buffer, char letter) {
		if (pstd::getAvaliableCount<char>(*buffer) == 0) {
			return 0;
		}
		pstd::FixedArray<char> letterArray{
			.allocation = pstd::arenaAlloc<char>(buffer, 1)
		};
		letterArray[0] = letter;
		return 1;
	}

	size_t pushString(pstd::FixedArena* buffer, const String& string) {
		pstd::Allocation allocation{
			pstd::arenaAlloc<char>(buffer, string.size)
		};

		memcpy(allocation.block, string.buffer, allocation.size);
		return allocation.size;
	}

	size_t pushStringUntilControlCharacter(
		pstd::FixedArena* buffer,
		const String& format,
		size_t* outFormatCharactersProccessed,
		char* outControlCharacter
	) {
		ASSERT(buffer);
		char res{};

		size_t searchSize{
			min(pstd::getAvaliableCount<char>(*buffer), format.size)
		};

		char previousLetter{};
		char controlCharacter{ '\0' };
		size_t normalStringSize{};
		size_t controlCharactersProccessedSize{};
		for (size_t i{}; i < searchSize; i++) {
			char currentLetter{ format.buffer[i] };

			if (previousLetter == '%') {
				controlCharacter = currentLetter;
				normalStringSize--;
				controlCharactersProccessedSize = 2;
				break;
			}

			previousLetter = currentLetter;
			normalStringSize++;
		}
		pstd::String normalString{ .buffer = format.buffer,
								   .size = normalStringSize };
		size_t stringSize{ pushString(buffer, normalString) };
		*outControlCharacter = controlCharacter;
		*outFormatCharactersProccessed =
			normalStringSize + controlCharactersProccessedSize;

		return stringSize;
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
