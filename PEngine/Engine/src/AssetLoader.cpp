#include "AssetLoader.h"
#include "Logging.h"
#include "STD/PArray.h"
#include "STD/PFileIO.h"
#include "STD/PAlgorithm.h"
#include "STD/PString.h"
#include "STD/PHashMap.h"

Engine::TextureData
	Engine::loadTexture(pstd::Arena* pArena, const pstd::String path) {
	pstd::Allocation rawTexture{ pstd::readFile(pArena, path) };

	Engine::TextureHeader* textureHeader{
		rcast<Engine::TextureHeader*>(rawTexture.block)
	};
	ASSERT(textureHeader->magic == Engine::TextureHeader::MAGIC);

	uint32_t* pixelArray{ rcast<uint32_t*>(textureHeader->data) };

	return Engine::TextureData{ .width = textureHeader->width,
								.height = textureHeader->height,
								.pPixels = pixelArray };
}

Engine::MeshData Engine::loadMesh(
	pstd::Arena* pArena, pstd::Arena scratchArena, const pstd::String path
) {
	pstd::Allocation rawMesh{ pstd::readFile(pArena, path) };

	Engine::MeshHeader* meshHeader{ rcast<Engine::MeshHeader*>(rawMesh.block) };
	ASSERT(meshHeader->magic == Engine::MeshHeader::MAGIC);

	auto* pIndices{ rcast<uint32_t*>(meshHeader->data) };
	auto* pPositions{ rcast<pstd::Vec3*>(pIndices + meshHeader->indexCount) };
	auto* pUVs{ rcast<pstd::Vec2*>(pPositions + meshHeader->vertexCount) };

	return Engine::MeshData{ .vertexCount = meshHeader->vertexCount,
							 .indexCount = meshHeader->indexCount,
							 .pIndices = pIndices,
							 .pPositions = pPositions,
							 .pUVs = pUVs };
}
