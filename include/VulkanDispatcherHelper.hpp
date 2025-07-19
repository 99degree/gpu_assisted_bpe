#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

class VulkanDispatcherHelper {
public:
    static void dispatchTokenizationLoop(
        VkDevice device,
        VkQueue computeQueue,
        VkCommandBuffer commandBuffer,
        VkPipeline pipeline,
        VkPipelineLayout pipelineLayout,
        VkDescriptorSet descriptorSet,
        VkDeviceMemory writeLenMem,
        VkDeviceMemory inputCursorMem,
        VkDeviceMemory inputCursorNextMem,
        VkDeviceMemory writeIndexMem,
        VkDeviceMemory atomicThreadIdMem,
        uint32_t inputLengthBytes,
        uint32_t maxTokens,
        uint32_t totalDictKeys
    );
};
