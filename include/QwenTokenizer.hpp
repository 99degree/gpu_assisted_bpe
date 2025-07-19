#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <map>

class QwenTokenizer {
public:
    explicit QwenTokenizer(const std::string& tokenizerPath);

    std::vector<uint32_t> encodeToBytes(std::string& text) const;
    const std::map<std::string, uint32_t>& vocab() const;

private:
    std::map<std::string, uint32_t> vocab_;
    void loadTokenizerJson(const std::string& path);
};
