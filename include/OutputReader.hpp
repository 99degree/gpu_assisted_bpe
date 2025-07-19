#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

/**
 * @brief OutputReader provides functions to read back data from GPU buffers.
 */
namespace OutputReader {

    /**
     * @brief Reads a buffer of uint32_t values from host-visible memory.
     * @param device Vulkan logical device
     * @param memory VkDeviceMemory bound to the output buffer
     * @param tokenCount Number of values to read (output tokens)
     * @return Vector of decoded token IDs
     */
    std::vector<uint32_t> readEncodedBuffer(
        VkDevice device,
        VkDeviceMemory memory,
        size_t tokenCount
    );
}
