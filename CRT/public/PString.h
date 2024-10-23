#pragma once
#include <stdint.h>
#include <PArena.h>

namespace pstd {
	struct String {
		const char* buffer;
		uint32_t size;
	};

	uint32_t getCStringLength(const char* cString);

	String createString(const char* cString);

	String uint32_tToString(Allocation buffer, uint32_t number);

	// String formatString(Allocation<char> buffer, String format);
	// String
	// 	formatString(Allocation<char> buffer, String format, uint32_t number);

	// template<typename T, typename... Args>
	// String formatString(
	// 	Allocation<char> buffer, String format, T val, Args... args
	// ) {
	// 	String intermediateFormat{ formatString(buffer, format, val) };
	// 	String formattedString{
	// 		formatString(buffer, intermediateFormat, args...)
	// 	};
	// 	formattedString.size += intermediateFormat.size;
	// 	return formattedString;
	// }
}  // namespace pstd
