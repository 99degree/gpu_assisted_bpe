#include "ShaderPipeline.hpp"
#include <fstream>
#include <vector>
#include <stdexcept>
#include <iostream>

#define VK_CHECK(x) \
    do { VkResult err = x; if (err != VK_SUCCESS) { \
        std::cerr << "Vulkan error: " << err << "\n"; std::exit(EXIT_FAILURE); } \
    } while (0)

void ShaderPipeline::initialize(VkDevice device, const std::string& spvPath, VkDescriptorSetLayout descriptorSetLayout) {
    createShaderModule(device, spvPath);
    createPipelineLayout(device, descriptorSetLayout);
    createComputePipeline(device);
}

void ShaderPipeline::destroy(VkDevice device) {
    if (pipeline) vkDestroyPipeline(device, pipeline, nullptr);
    if (pipelineLayout) vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    if (shaderModule) vkDestroyShaderModule(device, shaderModule, nullptr);
}

void ShaderPipeline::createShaderModule(VkDevice device, const std::string& spvPath) {
    std::ifstream file(spvPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) throw std::runtime_error("Failed to open SPIR-V file: " + spvPath);

    size_t size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(size);
    file.seekg(0);
    file.read(buffer.data(), size);

    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = buffer.size();
    info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VK_CHECK(vkCreateShaderModule(device, &info, nullptr, &shaderModule));
}

void ShaderPipeline::createPipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout) {
    // Push constants: struct Params { uint inputLength; uint maxTokens; uint totalDictKeys; }
    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushRange.offset = 0;
    pushRange.size = sizeof(uint32_t) * 3;

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushRange;

    VK_CHECK(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout));
}

void ShaderPipeline::createComputePipeline(VkDevice device) {
    VkPipelineShaderStageCreateInfo stage{};
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage.module = shaderModule;
    stage.pName = "main";

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = stage;
    pipelineInfo.layout = pipelineLayout;

    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
}
