#include "Camera.h"
#include "Engine.h"
#include "Core.h"
#include "EngineState.h"
#include "AssetCooker.h"
#include "STD/PArena.h"

int main() {
	pstd::AllocationRegistry allocRegistry{ pstd::createAllocationRegistry() };
	pstd::Arena primaryScratchArena{
		pstd::allocateArena(&allocRegistry, 1024 * 1024)
	};
	pstd::Arena secondaryScratchArena{
		pstd::allocateArena(&allocRegistry, 1024 * 1024)
	};

	cookBMP(primaryScratchArena, "assets\\textures\\Missing_Texture.bmp");
	cookOBJ(
		primaryScratchArena, secondaryScratchArena, "assets\\models\\cube.obj"
	);

	Engine::Subsystems subsystems{ Engine::startup(&allocRegistry) };
	Engine::State* pEngine{ subsystems.pEngine };
	while (Engine::tick(&allocRegistry, subsystems)) {}

	Engine::shutdown(subsystems);
}
