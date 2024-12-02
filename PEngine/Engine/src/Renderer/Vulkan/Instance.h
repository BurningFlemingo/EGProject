#pragma once
#include "PArena.h"
#include "PArray.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

VkInstance createInstance(pstd::ArenaFrame&& arenaFrame);
