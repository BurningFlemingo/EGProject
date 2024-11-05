#pragma once

#include "PString.h"
#include "PSTDAPI.h"

namespace pstd {
	PSTD_API bool consoleWrite(const String string);
	PSTD_API bool consoleWrite(const char* cString);
}  // namespace pstd
