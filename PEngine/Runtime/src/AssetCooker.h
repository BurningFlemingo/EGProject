#pragma once
#include "STD/PString.h"
#include "STD/PVector.h"
#include "STD/PArena.h"
#include "AssetLoader.h"

void cookBMP(
	pstd::Arena primaryScratchArena,
	pstd::Arena secondaryScratchArena,
	const pstd::String path
);
void cookOBJ(
	pstd::Arena primaryScratchArena,
	pstd::Arena secondaryScratchArena,
	const pstd::String path
);
