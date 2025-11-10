#include "STD/PArena.h"
#include "STD/PString.h"
#include "STD/PTypes.h"
#include "STD/PAlgorithm.h"
#include "STD/PArray.h"
#include "STD/PMath.h"
#include "STD/PMemory.h"
#include "Logging.h"

using namespace pstd;

namespace {
	pstd::String pushUInt64AsString(pstd::Arena* pArena, uint64_t number);
	pstd::String pushInt64AsString(pstd::Arena* pArena, int64_t number);
	pstd::String pushFloatAsString(
		pstd::Arena* pArena, float number, uint32_t precision = 5
	);
	pstd::String pushString(pstd::Arena* pArena, const String& string);
	pstd::String pushStringUntilControlCharacter(
		pstd::Arena* pArena,
		const String& format,
		uint32_t* outFormatCharactersProccessed,
		char* outControlCharacter
	);

	pstd::String pushLetter(pstd::Arena* pArena, char letter);
}  // namespace

String pstd::createString(pstd::Arena* pArena, const String& string) {
	ASSERT(pArena);

	char* newStringBuffer{ pstd::alloc<char>(pArena, string.size) };

	memcpy(newStringBuffer, string.buffer, string.size);

	return String{ .buffer = newStringBuffer, .size = string.size };
}

pstd::String pstd::createString(const pstd::Allocation& allocation) {
	return String{ .buffer = rcast<char*>(allocation.block),
				   .size = ncast<uint32_t>(allocation.size) };
}

String pstd::makeNullTerminated(pstd::Arena* pArena, String string) {
	ASSERT(pArena);
	ASSERT(string.buffer);
	if (string.size == 0) {
		return string;
	}

	uint32_t lastLetterIndex{ string.size - 1 };
	if (string.buffer[lastLetterIndex] == '\0') {
		return string;
	}

	uint32_t lettersToCopy{
		min(pstd::getAvailableCount<char>(*pArena), string.size)
	};

	char* newStringBuffer{ pstd::alloc<char>(pArena, lettersToCopy) };

	memcpy(newStringBuffer, string.buffer, lettersToCopy);
	pushLetter(pArena, '\0');
	return String{ .buffer = newStringBuffer, .size = lettersToCopy };
}

bool pstd::stringsMatch(const String& a, const String& b) {
	ASSERT(a.buffer);
	ASSERT(b.buffer);

	if (a.size != b.size) {
		return false;
	}
	return memcmp(a.buffer, b.buffer, a.size) == 0;
}

bool pstd::concat(String* a, String&& b) {
	ASSERT(a);
	ASSERT(b.buffer);
	if (!a->buffer) {
		*a = b;
		return false;
	}

	if (!b.buffer || !b.size) {
		return false;
	}

	const char* aEnd{ a->buffer + a->size };
	const char* bEnd{ b.buffer + b.size };
	uint32_t size{ a->size + b.size };
	if (aEnd == b.buffer) {
		*a = String{ .buffer = a->buffer, .size = size };
		return false;
	}
	return true;
}

String pstd::makeConcatted(pstd::Arena* pArena, String a, String b) {
	ASSERT(pArena);

	String newA{ pushString(pArena, a) };
	String newB{ pushString(pArena, b) };
	String res{
		.buffer = newA.buffer,
		.size = newA.size + newB.size,
	};

	return res;
}

String pstd::formatString(pstd::Arena* pArena, const String& format) {
	String res{ pushString(pArena, format) };
	return res;
}

template<typename T>
String pstd::formatString(pstd::Arena* pArena, const String& format, T val) {
	char controlCharacter{};
	uint32_t formatCharactersProccessed{};
	String string{ pushStringUntilControlCharacter(
		pArena, format, &formatCharactersProccessed, &controlCharacter
	) };

	switch (controlCharacter) {
		case 'i': {
			concat(&string, pushInt64AsString(pArena, ncast<int64_t>(val)));
		} break;
		case 'u': {
			concat(&string, pushInt64AsString(pArena, ncast<uint64_t>(val)));
		} break;
		case 'f': {
			concat(&string, pushFloatAsString(pArena, ncast<float>(val)));
		} break;
		default:
			ASSERT(false);
	}
	if (formatCharactersProccessed < format.size) {
		String restOfFormat{ .buffer =
								 format.buffer + formatCharactersProccessed,
							 .size = format.size - formatCharactersProccessed };

		concat(&string, pushString(pArena, restOfFormat));
	}

	return string;
}

template<>
String pstd::formatString(
	pstd::Arena* pArena, const String& format, pstd::String val
) {
	char controlCharacter{};
	uint32_t formatCharactersProccessed{};
	String string{ pushStringUntilControlCharacter(
		pArena, format, &formatCharactersProccessed, &controlCharacter
	) };

	if (controlCharacter == 'm' || controlCharacter == 's') {
		concat(&string, pushString(pArena, val));
	}

	if (formatCharactersProccessed < format.size) {
		String restOfFormat{ .buffer =
								 format.buffer + formatCharactersProccessed,
							 .size = format.size - formatCharactersProccessed };

		concat(&string, pushString(pArena, restOfFormat));
	}

	return string;
}

template<>
String pstd::formatString(
	pstd::Arena* pArena, const String& format, const char* val
) {
	return formatString(pArena, format, createString(val));
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

String pstd::getLine(String lines) {
	for (uint32_t i{}; i < lines.size; i++) {
		char ch{ lines.buffer[i] };
		if (ch == '\n') {
			return String{ .buffer = lines.buffer, .size = i + 1 };
		}
	}
	return lines;
}

pstd::Array<String>
	pstd::splitLine(pstd::Arena* pArena, String line, char seperator) {
	int nItems{ 1 };
	bool seperated{ false };
	for (size_t i{}; i < line.size; i++) {
		char ch{ line.buffer[i] };
		if (ch == seperator) {
			seperated = true;
			continue;
		}

		if (seperated) {
			nItems++;
			seperated = false;
		}
	}
	auto items{ pstd::createArray<String>(pArena, nItems, 0) };

	String item{ .buffer = line.buffer };
	seperated = false;
	for (size_t i{}; i < line.size; i++) {
		char ch{ line.buffer[i] };
		if (ch == seperator) {
			seperated = true;
			continue;
		}

		if (seperated) {
			pstd::pushBack(&items, item);
			item = String{ .buffer = line.buffer + i };
			seperated = false;
		}
		item.size++;
	}

	if (item.size > 0) {
		pstd::pushBack(&items, item);
	}

	return items;
}

float pstd::stringToFloat(String stringNum) {
	if (stringNum.size == 0) {
		return 0;
	}

	float wholePart{};
	float fractionalNumerator{};
	float fractionalDenominator{ 1 };
	bool wholePartDone{ false };
	bool isNegative{ false };

	if (stringNum.buffer[0] == '-') {
		isNegative = true;
	}

	for (int i{}; i < stringNum.size; i++) {
		char ch{ stringNum.buffer[i] };
		if (ch == '.' && wholePartDone == false) {
			wholePartDone = true;
			continue;
		}
		if (ch > '9' || ch < '0') {
			break;
		}

		float num{ ncast<float>(stringNum.buffer[i] - '0') };
		if (!wholePartDone) {
			wholePart *= 10;
			wholePart += num;
		} else {
			fractionalNumerator *= 10;
			fractionalDenominator *= 10;
			fractionalNumerator += num;
		}
	}

	float num{ wholePart + (fractionalNumerator / fractionalDenominator) };
	num = isNegative ? -num : num;

	return num;
}

namespace {
	String pushFloatAsString(
		pstd::Arena* pArena, float number, uint32_t precision
	) {
		ASSERT(precision < 128);  // to avoid a loop that blows out the buffer

		String string{};
		if (number < 0) {
			concat(&string, pushLetter(pArena, '-'));
			number *= -1.f;
		}

		auto wholePart{ ncast<uint32_t>(number) };
		concat(&string, pushUInt64AsString(pArena, wholePart));

		concat(&string, pushLetter(pArena, '.'));

		size_t factor{ pstd::pow<size_t>(10, precision) };
		auto decimalPart{
			ncast<uint32_t>((pstd::absf(number - wholePart) * factor) + 0.5f)
		};
		concat(&string, pushUInt64AsString(pArena, decimalPart));
		return string;
	}

	String pushInt64AsString(pstd::Arena* pArena, int64_t number) {
		size_t stringSize{};
		if (number >= 0) {
			return pushUInt64AsString(pArena, (uint32_t)number);
		}

		String string{ pushLetter(pArena, '-') };

		concat(&string, pushUInt64AsString(pArena, -number));
		return string;
	}

	String pushUInt64AsString(pstd::Arena* pArena, uint64_t number) {
		uint32_t count{ 1 };
		{
			size_t bufferCountAvaliable{ getAvailableCount<char>(*pArena) };
			uint64_t numberCopy{ number };
			while (numberCopy > 9 && count < bufferCountAvaliable) {
				numberCopy /= 10;
				count++;
			}
		}

		auto letterArray{ pstd::createArray<char>(pArena, count) };

		for (uint32_t i{}; i < count; i++) {
			uint32_t reverseIndex{ (count - 1) - i };
			uint64_t digit{ number % 10 };
			char digitLetter{ (char)((uint32_t)'0' + digit) };
			letterArray[reverseIndex] = digitLetter;
			number /= 10;
		}
		String string{ .buffer = rcast<const char*>(letterArray.data),
					   .size = ncast<uint32_t>(letterArray.count) };
		return string;
	}

	String pushLetter(pstd::Arena* pArena, char letter) {
		if (pstd::getAvailableCount<char>(*pArena) == 0) {
			return {};
		}

		auto letterArray{ pstd::createArray<char>(pArena, 1) };

		letterArray[0] = letter;
		String string{ .buffer = rcast<const char*>(letterArray.data),
					   .size = ncast<uint32_t>(letterArray.count) };
		return string;
	}

	String pushString(pstd::Arena* pArena, const String& string) {
		if (string.size == 0) {
			return {};
		}
		char* newStringBuffer{ pstd::alloc<char>(pArena, string.size) };

		memcpy(newStringBuffer, string.buffer, string.size);
		return String{ .buffer = newStringBuffer, .size = string.size };
	}

	String pushStringUntilControlCharacter(
		pstd::Arena* pArena,
		const String& format,
		uint32_t* outFormatCharactersProccessed,
		char* outControlCharacter
	) {
		size_t searchSize{
			min(pstd::getAvailableCount<char>(*pArena), format.size)
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
		*outControlCharacter = controlCharacter;
		*outFormatCharactersProccessed =
			normalStringSize + controlCharactersProccessedSize;

		String res{ pushString(pArena, normalString) };
		return res;
	}
}  // namespace

template String
	pstd::formatString(pstd::Arena* pArena, const String& format, uint64_t val);
template String
	pstd::formatString(pstd::Arena* pArena, const String& format, int64_t val);
template String
	pstd::formatString(pstd::Arena* pArena, const String& format, double val);

template String
	pstd::formatString(pstd::Arena* pArena, const String& format, uint32_t val);
template String
	pstd::formatString(pstd::Arena* pArena, const String& format, int32_t val);
template String
	pstd::formatString(pstd::Arena* pArena, const String& format, float val);
