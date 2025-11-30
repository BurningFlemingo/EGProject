#pragma once
#include "STD/PString.h"
#include "STD/PVector.h"
#include "STD/PArena.h"

void cookBMP(pstd::Arena scratchArena, const pstd::String path);
void cookOBJ(
	pstd::Arena primaryScratchArena,
	pstd::Arena secondaryScratchArena,
	const pstd::String path
);
