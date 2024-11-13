#include "include/PArena.h"
#include "include/PString.h"
#include "include/PTypes.h"
#include "include/PAlgorithm.h"
#include "include/PArray.h"
#include "include/PMath.h"
#include "include/PMemory.h"

using namespace pstd;

namespace {
	Allocation pushUInt64AsString(
		pstd::ArenaFrame&& frame, uint64_t number
	);	// returns size of string pushed
	Allocation pushInt64AsString(
		pstd::ArenaFrame&& frame, int64_t number
	);	// returns size of string pushed
	Allocation pushDoubleAsString(
		pstd::ArenaFrame&& frame,
		double number,
		uint32_t precision = 5
	);	// returns size of string pushed

	Allocation pushString(
		pstd::ArenaFrame&& frame, const String& string
	);	// returns size of string pushed

	Allocation pushStringUntilControlCharacter(
		pstd::ArenaFrame&& frame,
		const String& format,
		uint32_t* outFormatCharactersProccessed,
		char* outControlCharacter
	);
	// returns size of string pushed

	Allocation pushLetter(pstd::ArenaFrame&& frame, char letter);
}  // namespace

String pstd::createString(pstd::ArenaFrame&& arenaFrame, const String& string) {
	Allocation newStringAllocation{
		pstd::alloc<char>(&arenaFrame, string.size)
	};
	Allocation oldStringAllocation{ .block = rcast<uint8_t*>(string.buffer),
									.size = string.size };

	shallowCopy(&newStringAllocation, oldStringAllocation);
	String res{ .buffer = rcast<const char*>(newStringAllocation.block),
				.size = newStringAllocation.size };
	return res;
}

String pstd::makeNullTerminated(pstd::ArenaFrame&& arenaFrame, String string) {
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
	pstd::memCpy(
		rcast<void*>(stringAllocation.block), string.buffer, lettersToCopy
	);
	pushLetter(ArenaFrame{ arenaFrame.pArena, arenaFrame.state }, '\0');

	string = String{ .buffer = rcast<const char*>(stringAllocation.block),
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

String pstd::concat(pstd::ArenaFrame&& frame, String a, String b) {
	Allocation allocation{ pstd::coalesce(
		pushString({ frame.pArena, frame.state }, a),
		pushString({ frame.pArena, frame.state }, b)
	) };

	String res{ .buffer = cast<const char*>(allocation.block),
				.size = allocation.size };
	return res;
}

String pstd::formatString(pstd::ArenaFrame&& frame, const String& format) {
	ASSERT(buffer);

	Allocation allocation{ pushString({ frame.pArena, frame.state }, format) };

	String res{ .buffer = cast<const char*>(allocation.block),
				.size = allocation.size };
	return res;
}

template<typename T>
String
	pstd::formatString(pstd::ArenaFrame&& frame, const String& format, T val) {
	char controlCharacter{};
	uint32_t formatCharactersProccessed{};
	Allocation allocation{ pushStringUntilControlCharacter(
		{ frame.pArena, frame.state },
		format,
		&formatCharactersProccessed,
		&controlCharacter
	) };

	switch (controlCharacter) {
		case 'i': {
			allocation = coalesce(
				pushInt64AsString(
					{ frame.pArena, frame.state }, cast<int64_t>(val)
				),
				allocation
			);
		} break;
		case 'u': {
			allocation = coalesce(
				pushInt64AsString(
					{ frame.pArena, frame.state }, cast<uint64_t>(val)
				),
				allocation
			);
		} break;
		case 'f': {
			allocation = coalesce(
				pushInt64AsString(
					{ frame.pArena, frame.state }, cast<double>(val)
				),
				allocation
			);
		} break;
		default:
			break;
	}
	auto stringFormatSizeDifference{
		pstd::abs(sncast<int>(format.size) - sncast<int>(allocation.size))
	};

	if (formatCharactersProccessed < format.size) {
		String restOfFormat{ .buffer =
								 format.buffer + formatCharactersProccessed,
							 .size = format.size - formatCharactersProccessed };

		allocation = coalesce(
			pushString({ frame.pArena, frame.state }, restOfFormat), allocation
		);
	}

	String res{ .buffer = cast<const char*>(allocation.block),
				.size = allocation.size };
	return res;
}

template<>
String pstd::formatString(
	pstd::ArenaFrame&& frame, const String& format, pstd::String val
) {
	char controlCharacter{};
	uint32_t formatCharactersProccessed{};
	Allocation allocation{ pushStringUntilControlCharacter(
		{ frame.pArena, frame.state },
		format,
		&formatCharactersProccessed,
		&controlCharacter
	) };

	if (controlCharacter == 'm') {
		allocation = coalesce(
			pushString({ frame.pArena, frame.state }, val), allocation
		);
	}

	if (formatCharactersProccessed < format.size) {
		String restOfFormat{ .buffer =
								 format.buffer + formatCharactersProccessed,
							 .size = format.size - formatCharactersProccessed };

		allocation = coalesce(
			pushString({ frame.pArena, frame.state }, restOfFormat), allocation
		);
	}

	String res{ .buffer = cast<const char*>(allocation.block),
				.size = allocation.size };
	return res;
}

template<>
String pstd::formatString(
	pstd::ArenaFrame&& frame, const String& format, const char* val
) {
	return formatString(
		{ frame.pArena, frame.state }, format, createString(val)
	);
}

String pstd::getFileName(const String& string) {
	uint32_t fileNameSize{};
	for (uint32_t i{}; i < string.size; i++) {
		uint32_t reverseI{ string.size - 1 - i };

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
	const String& a, const String& b, uint32_t* outIndex
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
	const String& a, const String& b, uint32_t* outIndex
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
	Allocation pushDoubleAsString(
		pstd::ArenaFrame&& frame, double number, uint32_t precision
	) {
		ASSERT(precision < 128);  // to avoid a loop that blows out the buffer

		Allocation allocation{};
		if (number < 0) {
			allocation = coalesce(
				pushLetter({ frame.pArena, frame.state }, '-'), allocation
			);
			number *= -1.f;
		}

		auto wholePart{ sncast<uint32_t>(number) };
		allocation = coalesce(
			pushUInt64AsString({ frame.pArena, frame.state }, wholePart),
			allocation
		);

		allocation = coalesce(
			pushLetter({ frame.pArena, frame.state }, '.'), allocation
		);

		size_t factor{ pstd::pow<size_t>(10, precision) };
		auto decimalPart{
			sncast<uint32_t>((pstd::abs(number - wholePart) * factor) + 0.5f)
		};
		allocation = coalesce(
			pushUInt64AsString({ frame.pArena, frame.state }, decimalPart),
			allocation
		);
		return allocation;
	}

	Allocation pushInt64AsString(pstd::ArenaFrame&& frame, int64_t number) {
		size_t stringSize{};
		if (number >= 0) {
			return pushUInt64AsString(
				{ frame.pArena, frame.state }, (uint32_t)number
			);
		}

		Allocation allocation{ pushLetter({ frame.pArena, frame.state }, '-') };

		// this avoids overflow since |INT_MIN| = |INT_MAX| + 1
		uint32_t positiveNumber{ sncast<uint32_t>(-(number + 1)) + 1 };
		allocation = coalesce(
			pushUInt64AsString({ frame.pArena, frame.state }, positiveNumber),
			allocation
		);
		return allocation;
	}

	Allocation pushUInt64AsString(ArenaFrame&& frame, uint64_t number) {
		uint32_t count{ 1 };
		{
			size_t bufferCountAvaliable{ getAvailableCount<char>(frame) };
			uint64_t numberCopy{ number };
			while (numberCopy > 9 && count < bufferCountAvaliable) {
				numberCopy /= 10;
				count++;
			}
		}

		pstd::Array<char> letterArray{
			.allocation = alloc<char>(&frame, count),
		};

		for (uint32_t i{}; i < count; i++) {
			uint32_t reverseIndex{ (count - 1) - i };
			uint64_t digit{ number % 10 };
			char digitLetter{ (char)((uint32_t)'0' + digit) };
			letterArray[reverseIndex] = digitLetter;
			number /= 10;
		}
		return letterArray.allocation;
	}

	Allocation pushLetter(ArenaFrame&& frame, char letter) {
		if (pstd::getAvailableCount<char>(frame) == 0) {
			return {};
		}
		pstd::Array<char> letterArray{ .allocation = alloc<char>(&frame, 1) };
		letterArray[0] = letter;
		return letterArray.allocation;
	}

	Allocation pushString(ArenaFrame&& frame, const String& string) {
		pstd::Allocation allocation{ pstd::alloc<char>(&frame, string.size) };

		memcpy(rcast<void*>(allocation.block), string.buffer, allocation.size);
		return allocation;
	}

	Allocation pushStringUntilControlCharacter(
		pstd::ArenaFrame&& frame,
		const String& format,
		uint32_t* outFormatCharactersProccessed,
		char* outControlCharacter
	) {
		ASSERT(buffer);
		char res{};

		size_t searchSize{
			min(pstd::getAvailableCount<char>(frame), format.size)
		};

		char previousLetter{};
		char controlCharacter{ '\0' };
		uint32_t normalStringSize{};
		uint32_t controlCharactersProccessedSize{};
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
		Allocation allocation{
			pushString({ frame.pArena, frame.state }, normalString)
		};
		*outControlCharacter = controlCharacter;
		*outFormatCharactersProccessed =
			normalStringSize + controlCharactersProccessedSize;

		return allocation;
	}
}  // namespace

template String pstd::formatString(
	pstd::ArenaFrame&& frame, const String& format, uint64_t val
);
template String pstd::formatString(
	pstd::ArenaFrame&& frame, const String& format, int64_t val
);
template String pstd::formatString(
	pstd::ArenaFrame&& frame, const String& format, double val
);

template String pstd::formatString(
	pstd::ArenaFrame&& frame, const String& format, uint32_t val
);
template String pstd::formatString(
	pstd::ArenaFrame&& frame, const String& format, int32_t val
);
template String pstd::formatString(
	pstd::ArenaFrame&& frame, const String& format, float val
);
