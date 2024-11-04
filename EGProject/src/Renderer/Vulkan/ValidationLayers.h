#pragma once

#include "PArray.h"
#include "PArena.h"

pstd::FixedArray<const char*> findValidationLayers(
	pstd::FixedArena* layerArena, pstd::FixedArena scratchArena
);
