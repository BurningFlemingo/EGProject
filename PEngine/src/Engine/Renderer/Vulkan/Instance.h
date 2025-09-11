#pragma once
#include "Core/PArena.h"
#include "Core/PArray.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

VkInstance createInstance(pstd::ArenaPair scratchArenas);
