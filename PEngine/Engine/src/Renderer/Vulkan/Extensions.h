#pragma once

#include "PArray.h"

pstd::Array<const char*> getDebugExtensions();

pstd::Array<const char*> findExtensions(pstd::ArenaFrame&& arenaFrame);
