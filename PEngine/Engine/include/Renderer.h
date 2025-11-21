#pragma once
#include "STD/PMatrix.h"

namespace Renderer {
	struct State;

	void setMVPMatrix(State* pState, const pstd::Mat4& mvpMat);
}  // namespace Renderer
