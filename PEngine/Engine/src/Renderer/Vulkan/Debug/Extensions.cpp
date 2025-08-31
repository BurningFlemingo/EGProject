#include "Renderer/Vulkan/Extensions.h"

#include <vulkan/vulkan.h>

namespace {
	constexpr size_t NUMBER_OF_DEBUG_EXTENSIONS{ 1 };
	const char* g_DebugExtensions[NUMBER_OF_DEBUG_EXTENSIONS]{
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

}  // namespace

pstd::Array<const char*> getDebugExtensions() {
	return pstd::Array<const char*>{ .data = g_DebugExtensions,
									 .count = NUMBER_OF_DEBUG_EXTENSIONS };
}
