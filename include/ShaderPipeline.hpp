#pragma once

#include <vulkan/vulkan.h>
#include <string>

/**
 * @brief Manages loading, layout, and dispatch of a compute shader pipeline
 */
class ShaderPipeline {
public:
    /**
     * @brief Initialize pipeline by loading shader, creating layout, and setting up pipeline
     * @param device Vulkan logical device
     * @param spvPath Path to compiled SPIR-V shader (.spv)
     * @param descriptorSetLayout Layout of all bound buffers
     */
    void initialize(VkDevice device, const std::string& spvPath, VkDescriptorSetLayout descriptorSetLayout);

    /**
     * @brief Destroy all Vulkan handles (shader module, layout, pipeline)
     */
    void destroy(VkDevice device);

    // Public handles
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

private:
    VkShaderModule shaderModule = VK_NULL_HANDLE;

    void createShaderModule(VkDevice device, const std::string& spvPath);
    void createPipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout);
    void createComputePipeline(VkDevice device);
};
