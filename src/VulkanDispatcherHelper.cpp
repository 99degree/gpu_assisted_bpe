#include "VulkanDispatcherHelper.hpp"
#include "BufferHelper.hpp"
#include <chrono>
#include <iostream>

#define GROUP_COUNT 128

#define VK_CHECK(x) \
    do { VkResult err = x; if (err != VK_SUCCESS) { \
        std::cerr << __FILE__ << ":" << " L." << __LINE__ << " Vulkan error: " << err << "\n"; std::exit(EXIT_FAILURE); } \
    } while (0)

void VulkanDispatcherHelper::dispatchTokenizationLoop(
    VkDevice device,
    VkQueue computeQueue,
    VkCommandBuffer commandBuffer,
    VkPipeline pipeline,
    VkPipelineLayout pipelineLayout,
    VkDescriptorSet descriptorSet,
    VkDeviceMemory writeLenMem,
    VkDeviceMemory inputCursorMem,
    VkDeviceMemory inputCursorNextMem,
    VkDeviceMemory writeIndexMem,
    VkDeviceMemory atomicThreadIdMem,
    uint32_t inputLengthBytes,
    uint32_t maxTokens,
    uint32_t totalDictKeys
) {

    /* inputLengthBytes is becoming char -> int */
    struct Params {
        uint32_t inputLength;
        uint32_t maxTokens;
        uint32_t totalDictKeys;
    } pushConstants = { inputLengthBytes, maxTokens, totalDictKeys };

    uint32_t inputCursorVal = 0;
    uint32_t inputCursorNextVal = 0;
    uint32_t minVal = 1;
    uint32_t writeIndex = 0;
    uint32_t maxTokenCntVal = 0;
    uint32_t writeLenVal = 0;

    uint32_t displayCnt = 0;

    auto startTime = std::chrono::high_resolution_clock::now();

    VkCommandBufferBeginInfo begin{};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    std::cout << "inputLengthBytes: " << inputLengthBytes << "\n";

    BufferHelper::writeScalarToBuffer(device, atomicThreadIdMem, 0);

    while (inputCursorVal < inputLengthBytes) {
        BufferHelper::writeScalarToBuffer(device, inputCursorMem, inputCursorVal);
        BufferHelper::writeScalarToBuffer(device, inputCursorNextMem, inputCursorVal);
	BufferHelper::writeScalarToBuffer(device, writeLenMem, 0);
	BufferHelper::writeScalarToBuffer(device, writeIndexMem, writeIndex);

        vkResetCommandBuffer(commandBuffer, 0);
        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &begin));
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(Params), &pushConstants);

        uint32_t groupCount = std::min(minVal, ((maxTokens + GROUP_COUNT - 1) / GROUP_COUNT));
        vkCmdDispatch(commandBuffer, groupCount, 1, 1);
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(computeQueue);

        inputCursorNextVal = BufferHelper::readScalarFromBuffer<uint32_t>(device, inputCursorNextMem);
	writeLenVal = BufferHelper::readScalarFromBuffer<uint32_t>(device, writeLenMem);

	//if (writeLenVal > 0) {
        if (inputCursorNextVal == inputCursorVal) {
            inputCursorVal += 1;

	    if (displayCnt < 20)
		std::cout << "🔁 inputCursor = " << inputCursorVal << " missed 1 byte\n";
        } else {
	    if (displayCnt < 20) {
		displayCnt++;

		std::cout << "🔁 inputCursor = " << inputCursorVal
        	          << ", inputCursorNext = " << inputCursorNextVal << "\n";
	    }
            inputCursorVal = inputCursorNextVal + 1;
	    writeIndex += 1;
	    maxTokenCntVal += 1;
        }

	if (maxTokenCntVal > maxTokens)
	    break;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;
    std::cout << "⏱️ Dispatch time: " << elapsed.count() << " ms\n";
    std::cout << "✅ Tokenization complete.\n";
}
