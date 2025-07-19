#include "BufferHelper.hpp"
#include <stdexcept>
#include <cstring>
#include <iostream>

#define VK_CHECK(x) do { VkResult err = x; if (err != VK_SUCCESS) throw std::runtime_error("Vulkan error"); } while (0)

template <typename T>
void BufferHelper::createBufferFromVector(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    const std::vector<T>& data,
    VkBufferUsageFlags usage,
    VkBuffer& outBuffer,
    VkDeviceMemory& outMemory
) {
    size_t bufferSize = sizeof(T) * data.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &outBuffer));

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, outBuffer, &memReq);

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

    uint32_t typeIndex = 0;
    bool found = false;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((memReq.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            typeIndex = i;
            found = true;
            break;
        }
    }

    if (!found) throw std::runtime_error("❌ Suitable memory type not found");

    VkMemoryAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.allocationSize = memReq.size;
    alloc.memoryTypeIndex = typeIndex;
    VK_CHECK(vkAllocateMemory(device, &alloc, nullptr, &outMemory));
    VK_CHECK(vkBindBufferMemory(device, outBuffer, outMemory, 0));

    void* mapped = nullptr;
    VK_CHECK(vkMapMemory(device, outMemory, 0, bufferSize, 0, &mapped));
    std::memcpy(mapped, data.data(), bufferSize);
    vkUnmapMemory(device, outMemory);
}

template <typename T>
void BufferHelper::createScalarBuffer(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    T value,
    VkBufferUsageFlags usage,
    VkBuffer& outBuffer,
    VkDeviceMemory& outMemory
) {
    std::vector<T> vec = { value };
    createBufferFromVector(device, physicalDevice, vec, usage, outBuffer, outMemory);
}

void BufferHelper::writeScalarToBuffer(
    VkDevice device,
    VkDeviceMemory memory,
    uint32_t value
) {
    void* mapped = nullptr;
    VK_CHECK(vkMapMemory(device, memory, 0, sizeof(uint32_t), 0, &mapped));
    std::memcpy(mapped, &value, sizeof(uint32_t));
    vkUnmapMemory(device, memory);
}

template <typename T>
T BufferHelper::readScalarFromBuffer(
    VkDevice device,
    VkDeviceMemory memory
) {
    void* mapped = nullptr;
    VK_CHECK(vkMapMemory(device, memory, 0, sizeof(T), 0, &mapped));
    T value;
    std::memcpy(&value, mapped, sizeof(T));
    vkUnmapMemory(device, memory);
    return value;
}

// ✅ Explicit instantiations
template void BufferHelper::createBufferFromVector<uint32_t>(
    VkDevice, VkPhysicalDevice, const std::vector<uint32_t>&, VkBufferUsageFlags, VkBuffer&, VkDeviceMemory&);
template void BufferHelper::createBufferFromVector<uint64_t>(
    VkDevice, VkPhysicalDevice, const std::vector<uint64_t>&, VkBufferUsageFlags, VkBuffer&, VkDeviceMemory&);
template void BufferHelper::createBufferFromVector<int>(
    VkDevice, VkPhysicalDevice, const std::vector<int>&, VkBufferUsageFlags, VkBuffer&, VkDeviceMemory&);

template void BufferHelper::createScalarBuffer<uint32_t>(
    VkDevice, VkPhysicalDevice, uint32_t, VkBufferUsageFlags, VkBuffer&, VkDeviceMemory&);
template void BufferHelper::createScalarBuffer<uint64_t>(
    VkDevice, VkPhysicalDevice, uint64_t, VkBufferUsageFlags, VkBuffer&, VkDeviceMemory&);
template void BufferHelper::createScalarBuffer<int>(
    VkDevice, VkPhysicalDevice, int, VkBufferUsageFlags, VkBuffer&, VkDeviceMemory&);

template uint32_t BufferHelper::readScalarFromBuffer<uint32_t>(VkDevice, VkDeviceMemory);
template uint64_t BufferHelper::readScalarFromBuffer<uint64_t>(VkDevice, VkDeviceMemory);
template int BufferHelper::readScalarFromBuffer<int>(VkDevice, VkDeviceMemory);
