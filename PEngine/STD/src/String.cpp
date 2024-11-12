#include "include/PArena.h"
#include "include/PString.h"
#include "include/PTypes.h"
#include "include/PAlgorithm.h"
#include "include/PArray.h"
#include "include/PMath.h"
#include "include/PMemory.h"

using namespace pstd;

namespace {
	size_t pushUInt64AsString(
		pstd::FixedArenaFrame&& arenaFrame, uint64_t number
	);	// returns size of string pushed
	size_t pushInt64AsString(
		pstd::FixedArenaFrame&& arenaFrame, int64_t number
	);	// returns size of string pushed
	size_t pushDoubleAsString(
		pstd::FixedArenaFrame&& arenaFrame,
		double number,
		uint32_t precision = 5
	);	// returns size of string pushed

	size_t pushString(
		pstd::FixedArenaFrame&& arenaFrame, const String& string
	);	// returns size of string pushed

	size_t pushStringUntilControlCharacter(
		pstd::FixedArenaFrame&& arenaFrame,
		const String& format,
		size_t* outFormatCharactersProccessed,
		char* outControlCharacter
	);
	// returns size of string pushed

	size_t pushLetter(pstd::FixedArenaFrame&& arenaFrame, char letter);
}  // namespace

String pstd::createString(
	pstd::FixedArenaFrame&& arenaFrame, const String& string
) {
	Allocation newStringAllocation{
		pstd::alloc<char>(&arenaFrame, string.size)
	};
	Allocation oldStringAllocation{ .block = (void*)string.buffer,
									.size = string.size };

	shallowCopy(&newStringAllocation, oldStringAllocation);
	String res{ .buffer = (const char*)newStringAllocation.block,
				.size = newStringAllocation.size };
	return res;
}

String pstd::makeNullTerminated(
	pstd::FixedArenaFrame&& arenaFrame, String string
) {
	ASSERT(string.buffer);
	if (string.size == 0) {
		return string;
	}

	size_t lastLetterIndex{ string.size - 1 };
	if (string.buffer[lastLetterIndex] == '\0') {
		return string;
	}

	size_t lettersToCopy{
		min(pstd::getAvailableCount<char>(arenaFrame), string.size)
	};
	Allocation stringAllocation{
		pstd::alloc<char>(&arenaFrame, lettersToCopy)
	};
	pstd::memCpy(stringAllocation.block, string.buffer, lettersToCopy);
	pushLetter({ arenaFrame.pArena, arenaFrame.state }, '\0');

	string = String{ .buffer = (const char*)stringAllocation.block,
					 .size = stringAllocation.size };
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

String pstd::concat(pstd::FixedArenaFrame&& arenaFrame, String a, String b) {
	void* const stringAddress{ pstd::getNextAllocAddress<char>(*buffer) };

	size_t stringSize{ pushString(buffer, a) };
	stringSize += pushString(buffer, b);

	String res{ .buffer = (const char*)stringAddress, .size = stringSize };
	return res;
}

String pstd::formatString(
	pstd::FixedArenaFrame&& arenaFrame, const String& format
) {
	ASSERT(buffer);

	const void* stringAddress{ pstd::getNextAllocAddress<char>(*buffer) };
	size_t stringSize{ pushString(buffer, format) };

	String res{ .buffer = (const char*)stringAddress, .size = stringSize };
	return res;
}

template<typename T>
String pstd::formatString(
	pstd::FixedArenaFrame&& arenaFrame, const String& format, T val
) {
	ASSERT(buffer);
	const void* stringAddress{ pstd::getNextAllocAddress<char>(*buffer) };
	const size_t initialBufferCountAvaliable{
		pstd::getAvailableCount<char>(*buffer)
	};

	char controlCharacter{};
	size_t formatCharactersProccessed{};
	size_t stringSize{ pushStringUntilControlCharacter(
		buffer, format, &formatCharactersProccessed, &controlCharacter
	) };
	switch (controlCharacter) {
		case 'i': {
			stringSize += pushInt64AsString(buffer, (int64_t)val);
		} break;
		case 'u': {
			stringSize += pushUInt64AsString(buffer, (uint64_t)val);
		} break;
		case 'f': {
			stringSize += pushDoubleAsString(buffer, (double)val);
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
	pstd::FixedArenaFrame&& arenaFrame, const String& format, pstd::String val
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
	pstd::FixedArenaFrame&& arenaFrame, const String& format, const char* val
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

bool pstd::substringMatchForward(
	const String& a, const String& b, size_t* outIndex
) {
	ASSERT(b.size != 0);
	ASSERT(b.buffer);
	ASSERT(a.size != 0);
	ASSERT(a.buffer);

	size_t charactersMatched{};
	for (size_t i{}; i < a.size; i++) {
		if (a.buffer[i] == b.buffer[charactersMatched]) {
			charactersMatched++;
		} else {
			charactersMatched = 0;
		}

		if (charactersMatched == b.size) {
			if (outIndex) {
				*outIndex = i;
			}
			return true;
		}
	}
	return false;
}

bool pstd::substringMatchBackward(
	const String& a, const String& b, size_t* outIndex
) {
	ASSERT(b.size != 0);
	ASSERT(b.buffer);
	ASSERT(a.size != 0);
	ASSERT(a.buffer);

	size_t charactersMatched{};
	for (size_t i{}; i < a.size; i++) {
		size_t reverseIndexA{ a.size - i - 1 };

		size_t reverseIndexB{ b.size - charactersMatched - 1 };

		if (a.buffer[reverseIndexA] == b.buffer[reverseIndexB]) {
			charactersMatched++;
		} else {
			charactersMatched = 0;
		}

		if (charactersMatched == b.size) {
			if (outIndex) {
				*outIndex = reverseIndexA;
			}
			return true;
		}
	}
	return false;
}

namespace {
	size_t pushDoubleAsString(
		pstd::FixedArenaFrame&& arenaFrame, double number, uint32_t precision
	) {
		ASSERT(precision < 128);  // to avoid a loop that blows out the buffer
		ASSERT(buffer);

		size_t stringSize{};

		if (number < 0) {
			stringSize += pushLetter(buffer, '-');
			number *= -1.f;
		}

		uint32_t wholePart{ (uint32_t)number };
		stringSize += pushUInt64AsString(buffer, wholePart);

		stringSize += pushLetter(buffer, '.');

		size_t factor{ pstd::pow<size_t>(10, precision) };
		auto decimalPart{ (uint32_t)((pstd::abs(number - wholePart) * factor) +
									 0.5f) };
		stringSize += pushUInt64AsString(buffer, decimalPart);
		return stringSize;
	}

	size_t
		pushInt64AsString(pstd::FixedArenaFrame&& arenaFrame, int64_t number) {
		ASSERT(buffer);

		size_t stringSize{};
		if (number >= 0) {
			return pushUInt64AsString(buffer, (uint32_t)number);
		}

		stringSize += pushLetter(buffer, '-');

		// this avoids overflow since |INT_MIN| = |INT_MAX| + 1
		uint32_t positiveNumber{ (uint32_t)(-(number + 1)) + 1 };
		stringSize += pushUInt64AsString(buffer, positiveNumber);
		return stringSize;
	}

	size_t pushUInt64AsString(
		pstd::FixedArenaFrame&& arenaFrame, uint64_t number
	) {
		ASSERT(buffer);

		uint32_t count{ 1 };
		{
			size_t bufferCountAvaliable{ pstd::getAvailableCount<char>(*buffer
			) };
			uint64_t numberCopy{ number };
			while (numberCopy > 9 && count < bufferCountAvaliable) {
				numberCopy /= 10;
				count++;
			}
		}

		pstd::FixedArray<char> letterArray{
			.allocation = pstd::alloc<char>(buffer, count),
		};

		for (uint32_t i{}; i < count; i++) {
			uint32_t reverseIndex{ (count - 1) - i };
			uint64_t digit{ number % 10 };
			char digitLetter{ (char)((uint32_t)'0' + digit) };
			letterArray[reverseIndex] = digitLetter;
			number /= 10;
		}
		return letterArray.allocation.size;
	}

	size_t pushLetter(pstd::FixedArenaFrame&& arenaFrame, char letter) {
		if (pstd::getAvailableCount<char>(*buffer) == 0) {
			return 0;
		}
		pstd::FixedArray<char> letterArray{ .allocation =
												pstd::alloc<char>(buffer, 1) };
		letterArray[0] = letter;
		return 1;
	}

	size_t
		pushString(pstd::FixedArenaFrame&& arenaFrame, const String& string) {
		pstd::Allocation allocation{ pstd::alloc<char>(buffer, string.size) };

		memcpy(allocation.block, string.buffer, allocation.size);
		return allocation.size;
	}

	size_t pushStringUntilControlCharacter(
		pstd::FixedArenaFrame&& arenaFrame,
		const String& format,
		size_t* outFormatCharactersProccessed,
		char* outControlCharacter
	) {
		ASSERT(buffer);
		char res{};

		size_t searchSize{
			min(pstd::getAvailableCount<char>(*buffer), format.size)
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
	pstd::FixedArenaFrame&& arenaFrame, const String& format, uint64_t val
);
template String pstd::formatString(
	pstd::FixedArenaFrame&& arenaFrame, const String& format, int64_t val
);
template String pstd::formatString(
	pstd::FixedArenaFrame&& arenaFrame, const String& format, double val
);

template String pstd::formatString(
	pstd::FixedArenaFrame&& arenaFrame, const String& format, uint32_t val
);
template String pstd::formatString(
	pstd::FixedArenaFrame&& arenaFrame, const String& format, int32_t val
);
template String pstd::formatString(
	pstd::FixedArenaFrame&& arenaFrame, const String& format, float val
);
