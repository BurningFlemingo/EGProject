#pragma once
#include "STD/PArena.h"
#include "STD/PMemory.h"
#include "Engine.h"

namespace Engine {

	Subsystems startup();
	bool update(const Subsystems& state);
	void run(const Subsystems& state);
	void shutdown(const Subsystems& state);

}  // namespace Engine
