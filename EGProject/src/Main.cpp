#include "Logging.h"
#include "PArena.h"
#include "PArray.h"
#include "PCircularBuffer.h"

#include "PMatrix.h"
#include "PMemory.h"
#include "PVector.h"
#include "PMath.h"

#include "Platforms/Window.h"
#include "Logging.h"
#include "Renderer/Renderer.h"

int main() {
	// TODO: for possible alignment errors, find a better solution
	size_t allocPadding{ 100 };
	size_t subsystemAllocSize{ Platform::getSizeofState() };
	size_t vulkanInitializationSize{ 1024 };
	size_t totalApplicationSize{ allocPadding + subsystemAllocSize +
								 vulkanInitializationSize };

	pstd::FixedArena applicationArena{
		pstd::allocateFixedArena(totalApplicationSize)
	};

	size_t scratchSize{ 1024 };
	pstd::FixedArena scratchArena{ pstd::allocateFixedArena(scratchSize) };

	Platform::State platformState{
		Platform::startup(&applicationArena, "window", 1920 / 2, 1080 / 2)
	};

	Renderer::State rendererState{
		Renderer::startup(&applicationArena, scratchArena)
	};

	pstd::CircularBuffer<int> cBuf{
		.allocation{ pstd::arenaAlloc<int>(&applicationArena, 5) },
	};

	pstd::Vec2 myVec1{ .x = 1, .y = 2 };
	pstd::Vec2 myVec2{ .x = 2, .y = 3 };
	pstd::Vec2 vec{ myVec1 + myVec2 };
	pstd::Mat4 myMat{ pstd::getIdentityMatrix<4>() };

	pstd::Rot3 rotor{ pstd::calcRotor(pstd::UP, pstd::RIGHT, pstd::HALF_PI) };

	pstd::Vec3 myVec{ .y = 1.f, .z = 1.f };

	myVec = pstd::calcRotated(myVec, rotor);

	Console::log("x = %f \n", myVec.x);
	Console::log("y = %f \n", myVec.y);
	Console::log("z = %f \n", myVec.z);

	LOG_ERROR("my favorite number is %f cus its cool", -0.3f);

	pstd::pushBack(&cBuf, 1);
	pstd::pushBack(&cBuf, 2);
	pstd::pushBack(&cBuf, 3);
	pstd::pushBack(&cBuf, 4);
	pstd::pushBack(&cBuf, 5);
	pstd::pushBack(&cBuf, 6);

	volatile float test1{ pstd::atanf(0) };
	test1 = pstd::atanf(1);
	test1 = pstd::atanf(-1);
	test1 = pstd::atanf(10000);
	test1 = pstd::atanf(-10000);
	test1 = pstd::atanf(0.00001);

	bool isRunning{ true };
	while (isRunning && Platform::isRunning(platformState)) {
		Platform::update(platformState);

		pstd::FixedArray<KeyEvent> eventArray{
			Platform::popKeyEvents(platformState)
		};

		for (size_t i{}; i < pstd::getCapacity(eventArray); i++) {
			KeyEvent event{ eventArray[i] };
			if (event.action == InputAction::PRESSED) {
				if (event.code == InputCode::TAB) {
					isRunning = false;
				}
			}
		}
	}
	pstd::freeFixedArena(&applicationArena);
	Renderer::shutdown(rendererState);
	Platform::shutdown(platformState);
}
