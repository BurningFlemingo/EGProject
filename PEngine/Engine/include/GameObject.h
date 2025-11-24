#pragma once

#include "STD/PVector.h"
#include "STD/PString.h"

namespace Engine {
	struct Transform {
		pstd::Vec3 pos;
		pstd::Vec3 scale;
		pstd::Rot3 rot;
	};

	struct GameObject {
		pstd::String modelPath;
		Transform transform;
	};
}  // namespace Engine
