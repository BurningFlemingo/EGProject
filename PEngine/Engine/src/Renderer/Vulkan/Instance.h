#pragma once
#include "PArena.h"

#include <vulkan/vulkan.h>

VkInstance createInstance(pstd::ArenaFrame&& arenaFrame);
