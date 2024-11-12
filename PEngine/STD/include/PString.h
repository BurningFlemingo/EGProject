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

	String createString(FixedArenaFrame&& arenaFrame, const String& string);
	inline String
		createString(FixedArenaFrame&& arenaFrame, const char* string) {
		return createString(
			{ arenaFrame.pArena, arenaFrame.state }, createString(string)
		);
	}

	bool stringsMatch(const String& a, const String& b);

	String makeNullTerminated(FixedArenaFrame&& arenaFrame, String string);

	inline const char*
		createCString(FixedArenaFrame&& arenaFrame, const String& string) {
		return makeNullTerminated(
				   { arenaFrame.pArena, arenaFrame.state }, string
		)
			.buffer;
	}

	String concat(pstd::FixedArenaFrame&& buffer, String a, String b);

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

	String
		formatString(pstd::FixedArenaFrame&& arenaFrame, const String& format);

	template<typename T>
	String formatString(
		pstd::FixedArenaFrame&& arenaFrame, const String& format, T val
	);

	template<typename T, typename... Args>
	String formatString(
		pstd::FixedArenaFrame&& arenaFrame,
		const String& format,
		T val,
		Args... args
	) {
		String newFormat{ formatString(arenaFrame, format, val) };

		String res{ formatString(arenaFrame, newFormat, args...) };

		return res;
	}

	String getFileName(const String& string);

	String getFileName(const char* cString);

	inline bool stringsMatch(const char* a, const char* b) {
		return stringsMatch(createString(a), createString(b));
	}

	inline String
		formatString(pstd::FixedArenaFrame&& arenaFrame, const char* format) {
		return formatString(
			{ arenaFrame.pArena, arenaFrame.state }, createString(format)
		);
	}

	template<typename T>
	String formatString(
		pstd::FixedArenaFrame&& arenaFrame, const char* format, T val
	) {
		return formatString(
			{ arenaFrame.pArena, arenaFrame.state }, createString(format), val
		);
	}

	template<typename T, typename... Args>
	String formatString(
		pstd::FixedArenaFrame&& arenaFrame,
		const char* format,
		T val,
		Args... args
	) {
		return formatString(
			{ arenaFrame.pArena, arenaFrame.state },
			createString(format),
			val,
			args...
		);
	}
}  // namespace pstd
