#include "PString.h"
#include <stdint.h>

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
	pstd::String string{ .buffer = (char*)cString, .size = stringSize };
	return string;
}
