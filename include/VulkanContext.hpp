#pragma once

#include <vulkan/vulkan.h>
#include <string>

/**
 * @brief VulkanContext sets up the Vulkan instance, selects a physical device,
 *        creates a logical device, and exposes the compute queue.
 */
class VulkanContext {
public:
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue computeQueue;
    uint32_t computeQueueFamily;

    /**
     * @brief Initializes Vulkan instance, device, and compute queue
     */
    void initialize();

    /**
     * @brief Destroys Vulkan instance and device
     */
    void cleanup();

private:
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
};
