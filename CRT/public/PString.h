#pragma once
#include <stdint.h>

namespace Platform {
	struct String {
		char* buffer;
		uint32_t size;
	};

	uint32_t getCStringLength(const char* cString);

	String createString(const char* cString);
}  // namespace Platform
