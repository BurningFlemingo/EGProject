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

	String createString(FixedArena* arena, const String& string);
	inline String createString(FixedArena* arena, const char* string) {
		return createString(arena, createString(string));
	}

	bool stringsMatch(const String& a, const String& b);

	String makeNullTerminated(FixedArena* buffer, String string);

	inline const char* createCString(FixedArena* arena, const String& string) {
		return makeNullTerminated(arena, string).buffer;
	}

	String concat(pstd::FixedArena* buffer, String a, String b);

	bool substringMatchForward(
		const String& a, const String& b, size_t* outIndex = nullptr
	);
	inline bool substringMatchForward(
		const char* a, const char* b, size_t* outIndex = nullptr
	) {
		return substringMatchForward(
			createString(a), createString(b), outIndex
		);
	}

	bool substringMatchBackward(
		const String& a, const String& b, size_t* outIndex = nullptr
	);
	inline bool substringMatchBackward(
		const char* a, const char* b, size_t* outIndex = nullptr
	) {
		return substringMatchBackward(
			createString(a), createString(b), outIndex
		);
	}

	String formatString(pstd::FixedArena* buffer, const String& format);

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

	String getFileName(const String& string);

	String getFileName(const char* cString);

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
		return formatString(buffer, createString(format), val, args...);
	}
}  // namespace pstd
