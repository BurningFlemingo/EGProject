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

	uint32_t getCStringLength(const char* cString);

	String createString(const char* cString);

	template<typename T>
	String formatString(pstd::FixedArena* buffer, String format, T val);

	template<typename T>
	String formatString(pstd::FixedArena* buffer, const char* format, T val) {
		String stringFormat{ createString(format) };
		String res{ formatString(buffer, stringFormat, val) };
		return res;
	}
}  // namespace pstd
