#include "internal/Engine.h"

#include "include/Logging.h"

#include "Platforms/Window.h"
#include "Renderer/Renderer.h"

#include "PArena.h"
#include "PArray.h"
#include "PCircularBuffer.h"

#include "PMatrix.h"
#include "PMemory.h"
#include "PString.h"
#include "PVector.h"
#include "PMath.h"
#include "STD/internal/PMemory.h"
#include "STD/internal/PConsole.h"

#include <new>

namespace peng {
	namespace internal {
		struct State {
			pstd::Arena applicationArena;

			Platform::State* platformState;
			Renderer::State* rendererState;

			bool isRunning;
		};
	};	// namespace internal
}  // namespace peng

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

using namespace peng;
using namespace peng::internal;

size_t peng::internal::getSizeofState() {
	size_t totalSize{ sizeof(State) };
	return totalSize;
}

peng::internal::State* peng::internal::startup(
	pstd::AllocationRegistry* registry, pstd::ArenaFrame&& stateFrame
) {
	size_t scratchSize{ 1024 * 1024 };
	pstd::Arena subsystemArena{
		pstd::allocateArena(registry, getSizeofSubsystems() + scratchSize)
	};

	Platform::State* platformState{ Platform::startup(
		pstd::makeFrame(&subsystemArena), "window", 1920 / 2, 1080 / 2
	) };

	Renderer::State* rendererState{
		Renderer::startup(pstd::makeFrame(&subsystemArena), *platformState)
	};

	pstd::Allocation arenaAllocation{ pstd::alloc<State>(&stateFrame) };

	State* arenaPtr{ new (arenaAllocation.block)
						 State{ .applicationArena = subsystemArena,
								.platformState = platformState,
								.rendererState = rendererState,
								.isRunning = true } };
	return arenaPtr;
}
bool peng::internal::update(State* state) {
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
void peng::internal::shutdown(State* state) {
	Renderer::shutdown(state->rendererState);
	Platform::shutdown(state->platformState);
}
