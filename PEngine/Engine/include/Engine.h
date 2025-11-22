#pragma once
#include "STD/PMatrix.h"
#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "STD/PFileIO.h"

#include "Input.h"

#include "Renderer.h"

namespace Platform {
	struct State;
}

namespace Application {
	struct State;

}  // namespace Application

namespace Engine {
	struct Subsystems {
		Application::State* pApplicationState;
		Renderer::State* pRendererState;
		Platform::State* pPlatformState;
	};

	bool getPhysicalKeyDown(Application::State* pAppState, InputCode keyCode);
	bool getVirtualKeyDown(Application::State* pAppState, InputCode keyCode);
}  // namespace Engine
