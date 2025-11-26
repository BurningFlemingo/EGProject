#pragma once
#include <vulkan/vulkan.h>
#include "Device.h"

VkCommandBuffer beginTransientCmd(const Device& device, VkCommandPool pool);
void endTransientCmd(const Device& device, VkCommandBuffer cmdBuffer);
