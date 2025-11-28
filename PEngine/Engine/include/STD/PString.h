#pragma once

#include "PTypes.h"
#include "PMemory.h"
#include "PArena.h"
#include "PArray.h"

namespace pstd {
	struct String {
		String() = default;
		String(const String& string);
		String(const char* cString);
		String(const char* buf, uint32_t bufSize);

		const char& operator[](size_t index) const {
			ASSERT(buffer);
			ASSERT(size > index);

			return buffer[index];
		}

		const char* buffer{};
		uint32_t size{};
	};

	bool operator==(String a, String b);

	constexpr uint32_t getCStringLength(const char* cString) {
		constexpr uint32_t maxStringSize{
			1024 * 1024
		};	// to avoid an infinite loop for ill-formed cstrings

		uint32_t stringLength{};
		for (uint32_t i{}; i < maxStringSize; i++) {
			if (cString[i] == '\0') {
				return i;
			}
		}
		return 0;
	}

	inline String createString(const char* cString) {
		return pstd::String{ cString, getCStringLength(cString) };
	}

	String createString(Arena* pArena, const String& string);
	String createString(const Allocation& allocation);

	inline String createString(Arena* pArena, const char* string) {
		return createString(pArena, createString(string));
	}

	bool stringsMatch(const String& a, const String& b);

	String makeNullTerminated(Arena* pArena, String string);

	inline const char* createCString(Arena* pArena, const String& string) {
		return makeNullTerminated(pArena, string).buffer;
	}

	// strings must be right next to eachother in memory
	bool concat(String* a, String&& b);

	String makeConcatted(pstd::Arena* pArena, String a, String b);

	bool substringMatchForward(
		const String& a, const String& b, uint32_t* outIndex = nullptr
	);

	bool substringMatchForward(
		const char a, const String& b, uint32_t* outIndex = nullptr
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

	String formatString(pstd::Arena* pArena, const String& format);

	template<typename T>
	String formatString(pstd::Arena* pArena, const String& format, T val);

	template<typename T, typename... Args>
	String formatString(
		pstd::Arena* pArena, const String& format, T val, Args... args
	) {
		String newFormat{ formatString(pArena, format, val) };

		String res{ formatString(pArena, newFormat, args...) };

		return res;
	}

	String getFileName(const String& string);
	String getFileName(const char* cString);

	inline bool stringsMatch(const char* a, const char* b) {
		return stringsMatch(createString(a), createString(b));
	}

	inline String formatString(pstd::Arena* pArena, const char* format) {
		return formatString(pArena, createString(format));
	}

	template<typename T>
	String formatString(pstd::Arena* pArena, const char* format, T val) {
		return formatString(pArena, createString(format), val);
	}

	template<typename T, typename... Args>
	String formatString(
		pstd::Arena* pArena, const char* format, T val, Args... args
	) {
		return formatString(pArena, createString(format), val, args...);
	}

	String getLine(String lines);
	// consumes the line
	inline String readLine(String* pLines) {
		ASSERT(pLines);
		String line{ getLine(*pLines) };
		pLines->buffer += line.size;
		pLines->size -= line.size;
		return line;
	}
	pstd::Array<String>
		split(pstd::Arena* pArena, String line, String delimiters = " ");

	pstd::Array<String> split(
		pstd::Arena* pArena,
		String line,
		size_t maxItemCount,
		String delimiters = " "
	);

	float stringToFloat(String stringNum);

	template<typename T>
	T parse(String string);

	char readChar(String* pString);
	void trimLeading(String* string, const String& delimiters = " ");
	String readToken(String* string, const String& delimiters = " ");
	size_t countTokens(String string, const String& delimiters = " ");

	size_t hash(String string);
}  // namespace pstd
