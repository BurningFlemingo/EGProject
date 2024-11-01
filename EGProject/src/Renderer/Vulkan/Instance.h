#pragma once
#include <vulkan/vulkan.h>
#include "PArena.h"

VkInstance createInstance(pstd::FixedArena scratchArena);
