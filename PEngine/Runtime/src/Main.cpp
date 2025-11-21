#include "Engine.h"
#include "Core.h"
#include <Windows.h>

int main() {
	Engine::Subsystems subsystems{ Engine::startup() };
	Engine::run(subsystems);
	Engine::shutdown(subsystems);
}
