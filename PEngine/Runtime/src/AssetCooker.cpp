#include "AssetLoader.h"
#include "Logging.h"
#include "STD/PArray.h"
#include "STD/PFileIO.h"
#include "STD/PAlgorithm.h"
#include "STD/PString.h"
#include "STD/PHashMap.h"

#include "FileLoader.h"
#include <new>

namespace {

	void createDirectories(
		pstd::Arena scratchArena, pstd::String nestedDirectorys
	);

}  // namespace

void cookBMP(
	pstd::Arena primaryScratchArena,
	pstd::Arena secondaryScratchArena,
	const pstd::String path
) {
	Engine::TextureHeader* pTextureHeader{
		loadBMP(&primaryScratchArena, secondaryScratchArena, path)
	};

	pstd::String texturePath{ path };
	pstd::readLastToken(&texturePath, ".");
	pstd::readToken(
		&texturePath, "\\/"
	);	// to remove leading 'assets' directory

	texturePath =
		pstd::makeConcatted(&primaryScratchArena, "generated\\", texturePath);
	texturePath =
		pstd::makeConcatted(&primaryScratchArena, texturePath, "texture");

	createDirectories(primaryScratchArena, texturePath);

	pstd::FileHandle handle{ pstd::openFile(
		pstd::createCString(&primaryScratchArena, texturePath),
		pstd::FileAccess::write,
		pstd::FileAccess::none,
		pstd::FileCreate::createAlways
	) };
	pstd::writeFile(handle, pTextureHeader, pTextureHeader->fileSize);
	pstd::closeFile(handle);
}

void cookOBJ(
	pstd::Arena primaryScratchArena,
	pstd::Arena secondaryScratchArena,
	const pstd::String path
) {
	Engine::MeshHeader* pMeshHeader{
		loadOBJ(&primaryScratchArena, secondaryScratchArena, path)
	};

	pstd::String meshPath{ path };
	pstd::readLastToken(&meshPath, ".");
	pstd::readToken(&meshPath, "\\/");	// to remove leading 'assets' directory

	meshPath =
		pstd::makeConcatted(&primaryScratchArena, "generated\\", meshPath);
	meshPath = pstd::makeConcatted(&primaryScratchArena, meshPath, "mesh");

	createDirectories(primaryScratchArena, meshPath);

	pstd::FileHandle handle{ pstd::openFile(
		pstd::createCString(&primaryScratchArena, meshPath),
		pstd::FileAccess::write,
		pstd::FileAccess::none,
		pstd::FileCreate::createAlways
	) };
	pstd::writeFile(handle, pMeshHeader, pMeshHeader->fileSize);
	pstd::closeFile(handle);
}

namespace {
	void createDirectories(
		pstd::Arena scratchArena, pstd::String nestedDirectorys
	) {
		for (size_t i{}; i < nestedDirectorys.size; i++) {
			if (nestedDirectorys[i] == '\\') {
				pstd::String path(nestedDirectorys.buffer, i);
				pstd::createDirectory(pstd::createCString(&scratchArena, path));
			}
		}
	}
}  // namespace
