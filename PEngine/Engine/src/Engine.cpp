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
			pstd::FixedArena applicationArena;
			pstd::FixedArena scratchArena;

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

	constexpr size_t getSizeofScratchArena() {
		constexpr size_t scratchSize{ 1024 * 1024 * 1024 };
		return scratchSize;
	}
}  // namespace

using namespace peng;
using namespace peng::internal;

size_t peng::internal::getSizeofState() {
	size_t totalSize{ sizeof(State) };
	return totalSize;
}

peng::internal::State* peng::internal::startup(
	pstd::AllocationRegistry* registry, pstd::FixedArena* stateArena
) {
	// TODO: for possible alignment errors, find a better solution

	pstd::FixedArena applicationArena{
		pstd::allocateFixedArena(registry, getSizeofSubsystems())
	};

	pstd::FixedArena scratchArena{
		pstd::allocateFixedArena(registry, getSizeofScratchArena())
	};

	Platform::State* platformState{
		Platform::startup(&applicationArena, "window", 1920 / 2, 1080 / 2)
	};

	Renderer::State* rendererState{
		Renderer::startup(&applicationArena, scratchArena, *platformState)
	};

	pstd::Allocation arenaAllocation{ pstd::arenaAlloc<State>(stateArena) };

	State* arenaPtr{ new (arenaAllocation.block)
						 State{ .applicationArena = applicationArena,
								.scratchArena = scratchArena,
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
