#pragma once
#include <stdint.h>

namespace Platform {
	using State = void*;

	enum AllocationTypeFlags : uint32_t {
		ALLOC_TYPE_RESERVE = 0x1,
		ALLOC_TYPE_COMMIT = 0x2
	};

	const size_t getPlatformAllocSize();

	State startup(
		const char* windowName,
		const int windowX,
		const int windowY,
		const int windowWidth,
		const int windowHeight
	);
}  // namespace Platform
