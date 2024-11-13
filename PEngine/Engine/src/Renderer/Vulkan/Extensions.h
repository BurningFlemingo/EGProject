#pragma once

#include "PArray.h"

pstd::BoundedArray<const char*> getDebugExtensions();

pstd::Array<const char*> findExtensions(pstd::ArenaFrame&& arenaFrame);
