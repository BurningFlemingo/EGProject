#pragma once

#include "STD/PVector.h"

namespace Engine {
	struct Transform {
		pstd::Vec3 pos;
		pstd::Vec3 scale;
		pstd::Rot3 rot;
	};
}  // namespace Engine
