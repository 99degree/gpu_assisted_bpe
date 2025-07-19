#include "VulkanContext.hpp"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>

// Simple error check macro
#define VK_CHECK(x)                                                       \
    do {                                                                  \
        VkResult err = x;                                                 \
        if (err != VK_SUCCESS) {                                          \
            std::cerr << "Vulkan error: " << err                          \
                      << " at " << __FILE__ << ":" << __LINE__ << "\n";  \
            std::exit(EXIT_FAILURE);                                      \
        }                                                                 \
    } while (0)

void VulkanContext::initialize() {
    createInstance();
    pickPhysicalDevice();
    createLogicalDevice();
}

void VulkanContext::cleanup() {
    if (device) vkDestroyDevice(device, nullptr);
    if (instance) vkDestroyInstance(instance, nullptr);
}

void VulkanContext::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "QwenTokenizer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instInfo{};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;

    VK_CHECK(vkCreateInstance(&instInfo, nullptr, &instance));
}

void VulkanContext::pickPhysicalDevice() {
    uint32_t count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &count, nullptr));
    if (count == 0) throw std::runtime_error("No Vulkan-compatible GPU found");

    std::vector<VkPhysicalDevice> devices(count);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &count, devices.data()));
    physicalDevice = devices[0]; // First available device — customize if needed
}

void VulkanContext::createLogicalDevice() {
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> families(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());

    computeQueueFamily = 0;
    for (uint32_t i = 0; i < familyCount; ++i) {
        if (families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            computeQueueFamily = i;
            break;
        }
    }

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = computeQueueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;

    VK_CHECK(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device));
    vkGetDeviceQueue(device, computeQueueFamily, 0, &computeQueue);
}
