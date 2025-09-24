#include "Renderer/Vulkan/ValidationLayers.h"

#include "Logging.h"

#include "Core/PArray.h"
#include "Core/PArena.h"
#include "Core/PContainer.h"
#include "Core/PString.h"
#include "Core/PMemory.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

pstd::Array<const char*> findValidationLayers(pstd::Arena* pPersistArena) {
	uint32_t layerCount{};
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	auto layerProps{
		pstd::createArray<VkLayerProperties>(pPersistArena, layerCount)
	};

	vkEnumerateInstanceLayerProperties(&layerCount, layerProps.data);

	const char* requiredLayersBuffer[]{ "VK_LAYER_KHRONOS_validation" };
	auto requiredLayers{ pstd::createArray<const char*>(requiredLayersBuffer) };

	auto foundLayers{ pstd::createArray<const char*>(pPersistArena, 1, 0) };

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

	return foundLayers;
}
