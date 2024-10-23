#include "PString.h"
#include <stdint.h>
#include "PAlgorithm.h"

using namespace pstd;

uint32_t pstd::getCStringLength(const char* cString) {
	const uint32_t maxStringSize{ 1024 };
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

// TODO: im tired, ill refactor in the morning
String pstd::uint32_tToString(Allocation<char> buffer, uint32_t number) {
	uint32_t count{ 0 };
	uint32_t numberCopy{ number };
	while (numberCopy > 0 && count < buffer.size) {
		numberCopy /= 10;
		count++;
	}
	for (uint32_t i{ count }; i != 0; i--) {
		uint32_t digit{ number % 10 };
		char digitLetter{ (char)((uint32_t)'0' + digit) };
		buffer.block[i] = digitLetter;
		number /= 10;
	}

	String string{ .buffer = (const char*)buffer.block, .size = count };

	return { .buffer = (const char*)buffer.block, .size = 10 };
}

// String pstd::formatString(Allocation<char> buffer, String format) {
// 	return format;
// }
//

// String formatString(Allocation<char> buffer, String format, uint32_t number)
// { 	size_t searchSize{ pstd::min<size_t>(buffer.size, format.size) }; 	char
// previousLetter{}; 	uint32_t stringSize{}; 	for (size_t i{}; i < searchSize;
// i++) { 		stringSize++; 		char currentLetter{ format.buffer[i] };
// if (currentLetter != '%' && previousLetter != '%') { buffer.block[i] =
// currentLetter;
// 		}
// 		if (previousLetter == '%') {
// 			switch (currentLetter) {
// 				case 'i': {
// 					size_t parenthesisPosition{ i - 1 };
// 					stringSize +=
// 						uint32_tToString(
// 							{ .block = buffer.block += parenthesisPosition,
// 							  .size = buffer.size -= parenthesisPosition },
// 							number
// 						)
// 							.size;
// 				} break;
// 				default:
// 					break;
// 			}
// 		}
// 		previousLetter = currentLetter;
// 	}
// 	String newFormat{ .buffer = buffer.block, .size = stringSize };
// 	return { newFormat };
// }
