#include "Renderer/Vulkan/Extensions.h"
#include <vulkan/vulkan.h>

namespace {
	constexpr size_t NUMBER_OF_DEBUG_EXTENSIONS{ 1 };
	const char* g_DebugExtensions[NUMBER_OF_DEBUG_EXTENSIONS]{
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

}  // namespace

pstd::BoundedArray<const char*> getDebugExtensions() {
	return pstd::BoundedArray<const char*>{
		.allocation = { .block = g_DebugExtensions,
						.size =
							sizeof(const char*) * NUMBER_OF_DEBUG_EXTENSIONS },
		.count = NUMBER_OF_DEBUG_EXTENSIONS
	};
}
