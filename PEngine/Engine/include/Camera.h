#pragma once
#include "STD/PMatrix.h"
#include "GameObject.h"

namespace Renderer {
	struct Camera {
		Engine::Transform transform;
		float fovRadians;
		float nearPlane;
		float farPlane;
	};
	struct State;

	void setCamera(State* pState, const Camera& camera);

}  // namespace Renderer
