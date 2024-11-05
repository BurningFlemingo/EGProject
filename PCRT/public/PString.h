#pragma once

#include "PTypes.h"
#include "PMemory.h"
#include "PArena.h"
#include "PArray.h"
#include "PSTDAPI.h"

namespace pstd {
	struct String {
		const char* buffer;
		size_t size;
	};

	constexpr size_t getCStringLength(const char* cString) {
		if (cString == nullptr) {
			return 0;
		}

		constexpr uint32_t maxStringSize{
			1024
		};	// to avoid an infinite loop for ill-formed cstrings

		uint32_t stringLength{};
		while (stringLength < 1024 && *cString != '\0') {
			stringLength++;
			cString++;
		}

		return stringLength;
	}

	constexpr String createString(const char* cString) {
		size_t stringSize{ getCStringLength(cString) };
		pstd::String string{ .buffer = cString, .size = stringSize };
		return string;
	}
	PSTD_API bool stringsMatch(const String& a, const String& b);

	PSTD_API String makeNullTerminated(FixedArena* buffer, String string);

	PSTD_API String concat(pstd::FixedArena* buffer, String a, String b);

	PSTD_API String
		formatString(pstd::FixedArena* buffer, const String& format);

	template<typename T>
	String formatString(pstd::FixedArena* buffer, const String& format, T val);

	template<typename T, typename... Args>
	String formatString(
		pstd::FixedArena* buffer, const String& format, T val, Args... args
	) {
		String newFormat{ formatString(buffer, format, val) };

		String res{ formatString(buffer, newFormat, args...) };

		return res;
	}

	PSTD_API String getFileName(const String& string);

	PSTD_API String getFileName(const char* cString);

	inline bool stringsMatch(const char* a, const char* b) {
		return stringsMatch(createString(a), createString(b));
	}

	inline String formatString(pstd::FixedArena* buffer, const char* format) {
		return formatString(buffer, createString(format));
	}

	template<typename T>
	String formatString(pstd::FixedArena* buffer, const char* format, T val) {
		return formatString(buffer, createString(format), val);
	}

	template<typename T, typename... Args>
	String formatString(
		pstd::FixedArena* buffer, const char* format, T val, Args... args
	) {
		return formatString(buffer, createString(format), args...);
	}
}  // namespace pstd
