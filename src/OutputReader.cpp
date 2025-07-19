#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>

#define VK_CHECK(x) if ((x) != VK_SUCCESS) throw std::runtime_error("Vulkan error at line " + std::to_string(__LINE__));

namespace OutputReader {
    std::vector<uint32_t> readEncodedBuffer(
        VkDevice device,
        VkDeviceMemory outputMemory,
        size_t tokenCount
    ) {
        std::vector<uint32_t> output(tokenCount);
        VkDeviceSize size = tokenCount * sizeof(uint32_t);

        void* mapped;
        VK_CHECK(vkMapMemory(device, outputMemory, 0, size, 0, &mapped));
        std::memcpy(output.data(), mapped, size);
        vkUnmapMemory(device, outputMemory);

        return output;
    }
}
