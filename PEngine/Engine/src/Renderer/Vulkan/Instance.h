#pragma once
#include "STD/PArena.h"
#include "STD/PArray.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

VkInstance createInstance(pstd::Arena scratchArena1, pstd::Arena scratchArena2);
