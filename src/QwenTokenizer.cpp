#include "QwenTokenizer.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

QwenTokenizer::QwenTokenizer(const std::string& tokenizerPath) {
    loadTokenizerJson(tokenizerPath);
}

void QwenTokenizer::loadTokenizerJson(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("❌ Failed to open tokenizer file: " + path);
    }

    json j;
    in >> j;

    const auto& vocabJson = j["model"]["vocab"];
    for (auto it = vocabJson.begin(); it != vocabJson.end(); ++it) {
        vocab_[it.key()] = it.value();
    }

    // 🛑 Add special token "</s>" with reserved ID 0
    //vocab_["</s>"] = 0;
}

std::vector<uint32_t> QwenTokenizer::encodeToBytes(std::string& utf8Char) const {
    int fixedLength = 4;

    std::string str(utf8Char);
    // Convert UTF-8 string to UTF-32 code points
    std::u32string utf32 = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(str);

    std::vector<uint32_t> result(utf32.size(), 0);
    for (size_t i = 0; i < utf32.size(); ++i) {
        result[i] = static_cast<uint32_t>(utf32[i]);
    }
    return result;
}

const std::unordered_map<std::string, uint32_t>& QwenTokenizer::vocab() const {
    return vocab_;
}
