#include "Engine/Engine.h"

#include "Engine/Logging.h"

#include "Engine/Platforms/Window.h"
#include "Engine/Renderer/Renderer.h"

#include "Core/PArena.h"
#include "Core/PArray.h"
#include "Core/PCircularBuffer.h"

#include "Core/PMatrix.h"
#include "Core/PMemory.h"
#include "Core/PString.h"
#include "Core/PVector.h"
#include "Core/PMath.h"
#include "Core/PArray.h"
#include "Core/Memory.h"
#include "Core/Console.h"

#include "Core/PFunction.h"

#include <new>

namespace PE {
	struct State {
		pstd::Arena engineArena;

		Platform::State* platformState;
		Renderer::State* rendererState;

		bool isRunning;
	};
};	// namespace PE

namespace {
	size_t getSizeofSubsystems() {
		size_t allocPadding{ 100 };
		size_t platformSize{ Platform::getSizeofState() };
		size_t vulkanSize{ 1024 };
		size_t vulkanArraysSize{ 1024 };
		size_t totalSize{ allocPadding + platformSize + vulkanSize };
		return totalSize;
	}

}  // namespace

using namespace PE;

size_t PE::getSizeofState() {
	size_t totalSize{ sizeof(State) };
	return totalSize;
}

PE::State* PE::startup(
	pstd::AllocationRegistry* pRegistry,
	pstd::Arena* pPersistArena,
	pstd::ArenaPair scratchArenas
) {
	pstd::Arena subsystemArena{
		pstd::allocateArena(pRegistry, getSizeofSubsystems())
	};

	pstd::DArray<int> arr{ pstd::createDArray<int>(pRegistry, 1024) };

	Platform::State* platformState{
		Platform::startup(&subsystemArena, "window", 1920 / 2, 1080 / 2)
	};

	Renderer::State* rendererState{ Renderer::startup(
		&subsystemArena, scratchArenas, *platformState, pRegistry
	) };

	State* stateBlock{ pstd::alloc<State>(pPersistArena) };

	State* arenaPtr{ new (stateBlock) State{ .engineArena = subsystemArena,
											 .platformState = platformState,
											 .rendererState = rendererState,
											 .isRunning = true } };
	return arenaPtr;
}
bool PE::update(State* state) {
	if (state->isRunning && Platform::isRunning(state->platformState)) {
		Platform::update(state->platformState);

		Platform::Event event{};
		while (Platform::popEvent(state->platformState, &event)) {
			switch (event.type) {
				case Platform::EventType::key: {
					if (event.keyEvent.action == InputAction::PRESSED) {
						if (event.keyEvent.code == InputCode::TAB) {
							state->isRunning = false;
						}
					}
				} break;
				default:
					break;
			}
		}
	}
	if (state->isRunning) {
		return true;
	}
	return false;
}
void PE::shutdown(State* state) {
	Renderer::shutdown(state->rendererState);
	Platform::shutdown(state->platformState);
}
