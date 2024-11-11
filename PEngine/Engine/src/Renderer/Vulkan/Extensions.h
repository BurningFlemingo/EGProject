#pragma once

#include "PArray.h"

pstd::BoundedArray<const char*> getDebugExtensions();

pstd::FixedArray<const char*> findExtensions(pstd::FixedArenaFrame&& arenaFrame
);
