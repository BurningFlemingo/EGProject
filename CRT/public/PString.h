#pragma once
#include "PTypes.h"
#include "PMemory.h"
#include "PArena.h"
#include "PArray.h"

namespace pstd {
	struct String {
		const char* buffer;
		size_t size;
	};

	constexpr size_t getCStringLength(const char* cString) {
		if (cString == nullptr) {
			return 0;
		}

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

	constexpr String createString(const char* cString) {
		size_t stringSize{ getCStringLength(cString) };
		pstd::String string{ .buffer = cString, .size = stringSize };
		return string;
	}
	bool stringsMatch(const String& a, const String& b);

	String makeNullTerminated(FixedArena* buffer, const String& string);

	template<typename T>
	String formatString(pstd::FixedArena* buffer, const String& format, T val);

	String getFileName(const String& string);

	String getFileName(const char* cString);

	template<typename T>
	String formatString(pstd::FixedArena* buffer, const char* format, T val) {
		String stringFormat{ createString(format) };
		String res{ formatString(buffer, stringFormat, val) };
		return res;
	}
}  // namespace pstd
