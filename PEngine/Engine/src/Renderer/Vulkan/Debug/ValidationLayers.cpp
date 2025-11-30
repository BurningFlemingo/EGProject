#include "Renderer/Vulkan/ValidationLayers.h"

#include "Logging.h"

#include "STD/PArray.h"
#include "STD/PArena.h"
#include "STD/PContainer.h"
#include "STD/PString.h"
#include "STD/PMemory.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

pstd::Array<const char*> findValidationLayers(pstd::Arena* pPersistArena) {
	uint32_t layerCount{};
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	auto layerProps{
		pstd::createArray<VkLayerProperties>(pPersistArena, layerCount)
	};

	vkEnumerateInstanceLayerProperties(&layerCount, layerProps.data);

	pstd::StaticArray<pstd::String, 1> requiredLayers{
		"VK_LAYER_KHRONOS_validation",
	};

	auto foundLayers{
		pstd::createArray<const char*>(pPersistArena, requiredLayers.count, 0)
	};

	for (size_t i{}; i < requiredLayers.count; i++) {
		pstd::String queriedLayer{ requiredLayers[i] };
		bool layerFound{};
		for (size_t j{}; j < layerProps.count; j++) {
			const char* availableLayer{ layerProps[j].layerName };
			if (queriedLayer == availableLayer) {
				pstd::pushBack(&foundLayers, availableLayer);
				layerFound = true;
				break;
			}
		}
		if (layerFound) {
			LOG_INFO("found %m\n", queriedLayer);
		} else {
			LOG_WARN("could not find %m\n", queriedLayer);
		}
	}

	return foundLayers;
}
