#pragma once
#include "STD/PArena.h"
#include "AssetLoader.h"

Engine::MeshHeader* loadOBJ(
	pstd::Arena* pPersistArena,
	pstd::Arena scratchArena,
	const pstd::String path
);

Engine::TextureHeader* loadBMP(
	pstd::Arena* pPersistArena,
	pstd::Arena scratchArena,
	const pstd::String path
);
