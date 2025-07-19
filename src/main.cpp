#include "QwenTokenizer.hpp"
#include "VulkanContext.hpp"
#include "ShaderPipeline.hpp"
#include "BufferHelper.hpp"
#include "OutputReader.hpp"
#include "DescriptorHelper.hpp"
#include "DictionaryBuilder.hpp"
#include "VulkanDispatcherHelper.hpp"
#include "ByteAligner.hpp"

#include <vulkan/vulkan.h>
#include <chrono>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstdlib>
#include <algorithm>  // for std::min
#include <cstdint>    // for uint32_t

struct PushConstants {
    uint32_t inputLength;     // bytes
    uint32_t maxTokens;       // conservative output limit
    uint32_t totalDictKeys;   // vocabulary size
};

#define VK_CHECK(x) do { VkResult err = x; if (err != VK_SUCCESS) throw std::runtime_error("Vulkan error"); } while (0)

#define TOTAL_VULAKN_PARAM 12

int main(int argc, char* argv[]) {
    std::string prompt;
    uint limitedLength = -1;

    if (argc < 2) {
        std::cerr << "❌ Usage: " << argv[0] << " [-f filename] OR \"prompt text\"\n";
        return 1;
    }

    if (std::string(argv[1]) == "-f") {
        if (argc < 3) {
            std::cerr << "❌ Missing filename after '-f'\n";
            return 1;
        }
        std::ifstream in(argv[2]);
        if (!in) {
            std::cerr << "❌ Failed to open file: " << argv[2] << "\n";
            return 1;
        }
        std::getline(in, prompt, '\0'); // read entire file
        std::cout << "📁 Loaded prompt from file: \"" << argv[2] << "\"\n";
    } else if (std::string(argv[1]) == "-n") {
        if (argc < 4) {
            std::cerr << "❌ Missing length after '-n'\n";
            return 1;
        }
	limitedLength = atoi(argv[2]);

        prompt = argv[3];
        std::cout << "📝 Using direct prompt: \"" << prompt << "\"\n";

    } else {
        prompt = argv[1];
        std::cout << "📝 Using direct prompt: \"" << prompt << "\"\n";
    }

    try {
	prompt += "</s>";

	std::cout << "📝 Prompt: \"" << prompt << "\"\n";

        // 🔡 Tokenize prompt
        QwenTokenizer tokenizer("tokenizer.json");

	// 🔤 Encode input string
	std::vector<uint32_t> input32 = tokenizer.encodeToBytes(prompt);

        uint32_t inputLenBytes = input32.size();
        uint32_t maxTokens = inputLenBytes;  // safe upper bound

	std::cout << "inputLenBytes:" << inputLenBytes << " input32.size() " << input32.size() << "\n";

        // 📚 Build dictionary buffers
        DictionaryBuilder dictBuilder(tokenizer);
        //dictBuilder.build();

        // ⚙️ Initialize Vulkan
        VulkanContext vk;
        vk.initialize();

        // 📦 Allocate GPU buffers
        VkBuffer buffers[11];
        VkDeviceMemory memories[11];

	//input32
        BufferHelper::createBufferFromVector(vk.device, vk.physicalDevice, input32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[0], memories[0]);          // input32
	//dictkey
        BufferHelper::createBufferFromVector(vk.device, vk.physicalDevice, dictBuilder.dictKey(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[1], memories[1]);  // dictKey
	//dictlength
        BufferHelper::createBufferFromVector(vk.device, vk.physicalDevice, dictBuilder.dictLength(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[2], memories[2]); // dictLength
	//dictcode
        BufferHelper::createBufferFromVector(vk.device, vk.physicalDevice, dictBuilder.dictCode(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[3], memories[3]);   // dictCode
	//dictoffset
        BufferHelper::createBufferFromVector(vk.device, vk.physicalDevice, dictBuilder.dictOffset(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[4], memories[4]); // dictOffset

	// encoded
        BufferHelper::createBufferFromVector(vk.device, vk.physicalDevice, std::vector<uint32_t>(inputLenBytes), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[5], memories[5]); // encoded
	// writeLen
        BufferHelper::createScalarBuffer(vk.device, vk.physicalDevice, 0, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[6], memories[6]);  // writeLen
	//inputcursor
        BufferHelper::createScalarBuffer(vk.device, vk.physicalDevice, 0, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[7], memories[7]);  // inputCursor
	//next
        BufferHelper::createScalarBuffer(vk.device, vk.physicalDevice, 0, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[8], memories[8]);  // inputCursorNext
	//debug
	BufferHelper::createBufferFromVector(vk.device, vk.physicalDevice, std::vector<uint32_t>(dictBuilder.dictKey().size()), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[9], memories[9]);
	//writeidx
	BufferHelper::createScalarBuffer(vk.device, vk.physicalDevice, 0, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[10], memories[10]);  // writeIndex
	//atomicThreadID
	BufferHelper::createBufferFromVector(vk.device, vk.physicalDevice, std::vector<uint32_t>(dictBuilder.dictKey().size()), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffers[11], memories[11]); //atomicThreadID

        // 🧱 Descriptor layout + pipeline
        std::vector<VkDescriptorSetLayoutBinding> bindings(TOTAL_VULAKN_PARAM);
        for (uint32_t i = 0; i < TOTAL_VULAKN_PARAM; ++i) {
            bindings[i].binding = i;
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            bindings[i].descriptorCount = 1;
            bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        }

        VkPushConstantRange pushRange{};
        pushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pushRange.offset = 0;
        pushRange.size = sizeof(PushConstants);

std::cout << "L." <<  __LINE__ << "\n";

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkDescriptorSetLayout descriptorSetLayout;
        VK_CHECK(vkCreateDescriptorSetLayout(vk.device, &layoutInfo, nullptr, &descriptorSetLayout));

std::cout << "L." <<  __LINE__ << "\n";

        ShaderPipeline pipeline;
        pipeline.initialize(vk.device, "token_match.spv", descriptorSetLayout);

std::cout << "L." <<  __LINE__ << "\n";

        // 🧩 Descriptor pool + set
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet = createDescriptorResources(vk.device, descriptorSetLayout, descriptorPool, bindings.size());

std::cout << "L." <<  __LINE__ << "\n";

        // 🔗 Update descriptors
        VkDescriptorBufferInfo infos[TOTAL_VULAKN_PARAM];
        for (int i = 0; i < TOTAL_VULAKN_PARAM; ++i)
            infos[i] = { buffers[i], 0, VK_WHOLE_SIZE };

std::cout << "L." <<  __LINE__ << "\n";

        std::vector<VkWriteDescriptorSet> writes(TOTAL_VULAKN_PARAM);
        for (int i = 0; i < TOTAL_VULAKN_PARAM; ++i) {
            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet = descriptorSet;
            writes[i].dstBinding = i;
            writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writes[i].descriptorCount = 1;
            writes[i].pBufferInfo = &infos[i];
        }
        vkUpdateDescriptorSets(vk.device, TOTAL_VULAKN_PARAM, writes.data(), 0, nullptr);

std::cout << "L." <<  __LINE__ << "\n";

        // 🧨 Command buffer setup
        VkCommandPool cmdPool;
        {
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = vk.computeQueueFamily;
            VK_CHECK(vkCreateCommandPool(vk.device, &poolInfo, nullptr, &cmdPool));
        }


        VkCommandBuffer cmdBuf;
        {
            VkCommandBufferAllocateInfo alloc{};
            alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc.commandPool = cmdPool;
            alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc.commandBufferCount = 1;
            VK_CHECK(vkAllocateCommandBuffers(vk.device, &alloc, &cmdBuf));
        }

        //VkCommandBufferBeginInfo begin{};
        //begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //VK_CHECK(vkBeginCommandBuffer(cmdBuf, &begin));

	// 🔁 Dispatch loop
	VulkanDispatcherHelper::dispatchTokenizationLoop(
		vk.device,
		vk.computeQueue,
		cmdBuf,
		pipeline.pipeline,
		pipeline.pipelineLayout,
		descriptorSet,
		memories[6], //writeLen
		memories[7], //inputcur
		memories[8], //inputcurnext
		memories[10], //writeidx
                memories[11], //threadit
		std::min(static_cast<uint32_t>(limitedLength), static_cast<uint32_t>(input32.size())), /* input32 byte aligned */
		static_cast<uint32_t>(input32.size()), /* assume same to input length, maxTokens generated */
		static_cast<uint32_t>(dictBuilder.dictKey().size())
	);

        // 📤 Read back encoded output
        std::vector<uint32_t> encoded = OutputReader::readEncodedBuffer(vk.device, memories[5], maxTokens);
        std::cout << "🧠 Encoded token results:\n";
        for (size_t i = 0; i < encoded.size(); ++i) {
            if (encoded[i] != 0) {
                std::cout << "[" << i << "] → TokenID: " << (encoded[i]) << " Text: " << dictBuilder.getTokenById(encoded[i]) << "\n";
		if(encoded[i] != i)
			std::cout << "Error found!\n";
	    }
        }
#if 1
	std::vector<uint32_t> debugOut = OutputReader::readEncodedBuffer(vk.device, memories[9], dictBuilder.dictKey().size());
	for (size_t i = 0; i < 256; ++i) {
		uint32_t val = debugOut[i];
		bool matched = val >> 31;
		uint16_t length = (val >> 16) & 0xFF;
		uint16_t dictIndex = val & 0xFFFF;
		uint16_t threadID = (val >> 24) & 0xfF;
		bool shorter = (val >> 30) & 0x1;

		if (matched) {
		    std::cout << "Thread " << i << ": ✅ match length " << length <<
			", dict index " << dictIndex << " shorter " << shorter <<
			" custom tid " << threadID << "\n";
		}
	}

	std::vector<uint32_t> outThreadID = OutputReader::readEncodedBuffer(vk.device, memories[11], dictBuilder.dictKey().size());
	for (size_t i = 0; i < 256; ++i) {
		std::cout << "Thread " << i << " with "<< outThreadID[i] << " invoked times\n";

	}
#endif
        // 🧹 Cleanup
        pipeline.destroy(vk.device);
        vkDestroyDescriptorSetLayout(vk.device, descriptorSetLayout, nullptr);
        vkDestroyDescriptorPool(vk.device, descriptorPool, nullptr);
        vkDestroyCommandPool(vk.device, cmdPool, nullptr);
        vk.cleanup();

    } catch (const std::exception& e) {
        std::cerr << "💥 Crash: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
