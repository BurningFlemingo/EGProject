#include "Renderer/Vulkan/ValidationLayers.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "PArray.h"
#include "PArena.h"
#include "PContainer.h"
#include "PString.h"
#include "Logging.h"
#include "PMemory.h"

pstd::FixedArray<const char*> findValidationLayers(
	pstd::FixedArena* layersArena, pstd::FixedArena scratchArena
) {
	uint32_t layerCount{};
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	pstd::FixedArray<VkLayerProperties> layerProps{
		.allocation =
			pstd::arenaAlloc<VkLayerProperties>(layersArena, layerCount),
	};
	vkEnumerateInstanceLayerProperties(
		&layerCount, (VkLayerProperties*)layerProps.allocation.block
	);

	pstd::BoundedStackArray<const char*, 1> requiredLayers{
		.data = { "VK_LAYER_KHRONOS_validation" }, .count = 1
	};

	pstd::BoundedStackArray<const char*, 1> foundLayers{};

	for (int i{}; i < layerCount; i++) {
		if (requiredLayers.count == 0) {
			break;
		}
		char* avaliableLayer{ layerProps[i].layerName };
		size_t foundIndex{};
		auto matchFunction{ [&](const char* requiredLayer) {
			return pstd::stringsMatch(avaliableLayer, requiredLayer);
		} };

		if (pstd::find(requiredLayers, matchFunction, &foundIndex)) {
			pstd::pushBack(
				&foundLayers, requiredLayers[foundIndex]
			);	// ptr to string literal
			pstd::compactRemove(&requiredLayers, foundIndex);
		}
	}
	for (int i{}; i < requiredLayers.count; i++) {
		LOG_ERROR("could not find %m\n", requiredLayers[i]);
	}

	for (int i{}; i < foundLayers.count; i++) {
		LOG_INFO("found %m\n", foundLayers[i]);
	}

	pstd::FixedArray<const char*> res{
		.allocation =
			pstd::arenaAlloc<const char*>(layersArena, foundLayers.count),
	};
	pstd::shallowMove(&res.allocation, pstd::getStackAllocation(foundLayers));
	return res;
}
