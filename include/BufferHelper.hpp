#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

class BufferHelper {
public:
    // Create a GPU buffer from a vector of data
    template <typename T>
    static void createBufferFromVector(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        const std::vector<T>& data,
        VkBufferUsageFlags usage,
        VkBuffer& outBuffer,
        VkDeviceMemory& outMemory
    );

    // Create a GPU buffer from a single scalar value
    template <typename T>
    static void createScalarBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        T value,
        VkBufferUsageFlags usage,
        VkBuffer& outBuffer,
        VkDeviceMemory& outMemory
    );

    // Write a scalar value into an existing mapped buffer
    static void writeScalarToBuffer(
        VkDevice device,
        VkDeviceMemory memory,
        uint32_t value
    );

    // Read a scalar value from a mapped buffer
    template <typename T>
    static T readScalarFromBuffer(
        VkDevice device,
        VkDeviceMemory memory
    );
};
