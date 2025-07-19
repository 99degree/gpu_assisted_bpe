#include "DictionaryBuilder.hpp"
#include "QwenTokenizer.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

DictionaryBuilder::DictionaryBuilder(const QwenTokenizer& tokenizer) {
    const auto& vocab = tokenizer.vocab();

    std::cout << "📚 DictionaryBuilder: Packing tokens\n";

    size_t count = 0;
    for (const auto& [token, id] : vocab) {
	std::string str(token);
        std::vector<uint32_t> aligned = tokenizer.encodeToBytes(str);
        dictOffset_.push_back(dictKey_.size());
        dictLength_.push_back(static_cast<uint32_t>(aligned.size()));
        dictCode_.push_back(id);
        dictKey_.insert(dictKey_.end(), aligned.begin(), aligned.end());

        // 🧪 Print only first 20 tokens
        if (count < 20) {
            std::cout << "  [" << count << "] Token: \"" << token << "\"\n";
            std::cout << "    Code: " << id << ", Length: " << aligned.size() << "\n";
            std::cout << "    Aligned: ";
            for (uint32_t u : aligned) std::cout << u << " ";
            std::cout << "\n";
        }
        ++count;
    }

    std::cout << "📦 Total tokens packed: " << dictCode_.size() << "\n";
}

std::string DictionaryBuilder::getTokenById(uint32_t id) const {
    for (size_t i = 0; i < dictCode_.size(); ++i) {
        if (dictCode_[i] == id) {
            uint32_t offset = dictOffset_[i];
            uint32_t length = dictLength_[i];

            std::string token;
            for (uint32_t j = 0; j < length; ++j) {
                token += static_cast<char>(dictKey_[offset + j]);
            }
            return token;
        }
    }
    return "<unknown>";
}

const std::vector<uint32_t>& DictionaryBuilder::dictKey() const { return dictKey_; }
const std::vector<uint32_t>& DictionaryBuilder::dictOffset() const { return dictOffset_; }
const std::vector<uint32_t>& DictionaryBuilder::dictLength() const { return dictLength_; }
const std::vector<uint32_t>& DictionaryBuilder::dictCode() const { return dictCode_; }
