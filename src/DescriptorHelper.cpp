#include "DescriptorHelper.hpp"
#include <stdexcept>
#include <iostream>

#define VK_CHECK(x) \
    do { VkResult err = x; if (err != VK_SUCCESS) { \
        std::cerr << "Vulkan error: " << err << "\n"; std::exit(EXIT_FAILURE); } \
    } while (0)

/**
 * @brief Creates descriptor pool and allocates a descriptor set with 9 storage buffer bindings
 */
VkDescriptorSet createDescriptorResources(
    VkDevice device,
    VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorPool& outPool,
    uint32_t bindingCount // ✅ new parameter
) {
    // 🧱 Create descriptor pool for 9 bindings
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = bindingCount;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &outPool));

    // 🎮 Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = outPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VkDescriptorSet descriptorSet;
    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

    return descriptorSet;
}
