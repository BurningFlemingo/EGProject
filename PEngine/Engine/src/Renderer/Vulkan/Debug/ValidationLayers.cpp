#include "Renderer/Vulkan/ValidationLayers.h"

#include "include/Logging.h"

#include "PArray.h"
#include "PArena.h"
#include "PContainer.h"
#include "PString.h"
#include "PMemory.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

pstd::Array<const char*> findValidationLayers(pstd::Arena* pPersistArena) {
	uint32_t layerCount{};
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	pstd::Array<VkLayerProperties> layerProps{
		.allocation = pstd::alloc<VkLayerProperties>(pPersistArena, layerCount),
	};
	vkEnumerateInstanceLayerProperties(
		&layerCount, (VkLayerProperties*)layerProps.allocation.block
	);

	pstd::BoundedStaticArray<const char*, 1> requiredLayers{
		.data = { "VK_LAYER_KHRONOS_validation" }, .count = 1
	};

	pstd::BoundedArray<const char*> foundLayers{
		.allocation = pstd::alloc<const char*>(pPersistArena, 1)
	};

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

	pstd::Array<const char*> res{ .allocation = foundLayers.allocation };
	return res;
}
