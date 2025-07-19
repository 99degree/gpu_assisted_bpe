// DescriptorHelper.hpp
#pragma once
#include <vulkan/vulkan.h>

/**
 * @brief Creates descriptor pool and allocates descriptor set for 3 storage buffers.
 * @param device Vulkan logical device
 * @param setLayout Predefined descriptor set layout
 * @return VkDescriptorSet bound to a new descriptor pool
 */
VkDescriptorSet createDescriptorResources(
    VkDevice device,
    VkDescriptorSetLayout setLayout,
    VkDescriptorPool& outPool,
    uint32_t bindingCount // ✅ new parameter
);
