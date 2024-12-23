#pragma once

#include "PTypes.h"
#include "PMemory.h"
#include "PArena.h"
#include "PArray.h"

namespace pstd {
	struct String {
		const char* buffer;
		uint32_t size;
	};

	constexpr uint32_t getCStringLength(const char* cString) {
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
		uint32_t stringSize{ getCStringLength(cString) };
		pstd::String string{ .buffer = cString, .size = stringSize };
		return string;
	}

	String createString(ArenaFrame&& arenaFrame, const String& string);
	inline String createString(ArenaFrame&& arenaFrame, const char* string) {
		return createString(
			pstd::makeFrame(arenaFrame, arenaFrame.pPersistOffset),
			createString(string)
		);
	}

	bool stringsMatch(const String& a, const String& b);

	String makeNullTerminated(ArenaFrame&& arenaFrame, String string);

	inline const char*
		createCString(ArenaFrame&& arenaFrame, const String& string) {
		return makeNullTerminated(
				   pstd::makeFrame(arenaFrame, arenaFrame.pPersistOffset),
				   string
		)
			.buffer;
	}

	// strings must be right next to eachother in memory
	bool concat(String* a, String&& b);

	String makeConcatted(pstd::ArenaFrame&& buffer, String a, String b);

	bool substringMatchForward(
		const String& a, const String& b, uint32_t* outIndex = nullptr
	);
	inline bool substringMatchForward(
		const char* a, const char* b, uint32_t* outIndex = nullptr
	) {
		return substringMatchForward(
			createString(a), createString(b), outIndex
		);
	}

	bool substringMatchBackward(
		const String& a, const String& b, uint32_t* outIndex = nullptr
	);
	inline bool substringMatchBackward(
		const char* a, const char* b, uint32_t* outIndex = nullptr
	) {
		return substringMatchBackward(
			createString(a), createString(b), outIndex
		);
	}

	String formatString(pstd::ArenaFrame&& arenaFrame, const String& format);

	template<typename T>
	String formatString(
		pstd::ArenaFrame&& arenaFrame, const String& format, T val
	);

	template<typename T, typename... Args>
	String formatString(
		pstd::ArenaFrame&& arenaFrame, const String& format, T val, Args... args
	) {
		String newFormat{ formatString(
			pstd::makeFrame(arenaFrame, arenaFrame.pPersistOffset), format, val
		) };

		String res{ formatString(
			pstd::makeFrame(arenaFrame, arenaFrame.pPersistOffset),
			newFormat,
			args...
		) };

		return res;
	}

	String getFileName(const String& string);

	String getFileName(const char* cString);

	inline bool stringsMatch(const char* a, const char* b) {
		return stringsMatch(createString(a), createString(b));
	}

	inline String
		formatString(pstd::ArenaFrame&& arenaFrame, const char* format) {
		return formatString(
			pstd::makeFrame(arenaFrame, arenaFrame.pPersistOffset),
			createString(format)
		);
	}

	template<typename T>
	String
		formatString(pstd::ArenaFrame&& arenaFrame, const char* format, T val) {
		return formatString(
			pstd::makeFrame(arenaFrame, arenaFrame.pPersistOffset),
			createString(format),
			val
		);
	}

	template<typename T, typename... Args>
	String formatString(
		pstd::ArenaFrame&& arenaFrame, const char* format, T val, Args... args
	) {
		return formatString(
			pstd::makeFrame(arenaFrame, arenaFrame.pPersistOffset),
			createString(format),
			val,
			args...
		);
	}
}  // namespace pstd
