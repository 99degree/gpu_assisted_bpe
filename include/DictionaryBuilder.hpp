#pragma once
#include "QwenTokenizer.hpp"
#include <vector>
#include <unordered_map>

class DictionaryBuilder {
public:
    explicit DictionaryBuilder(const QwenTokenizer& tokenizer);

    const std::vector<uint32_t>& dictKey() const;
    const std::vector<uint32_t>& dictOffset() const;
    const std::vector<uint32_t>& dictLength() const;
    const std::vector<uint32_t>& dictCode() const;

    std::string getTokenById(uint32_t id) const;

private:
    std::vector<uint32_t> dictKey_;
    std::vector<uint32_t> dictOffset_;
    std::vector<uint32_t> dictLength_;
    std::vector<uint32_t> dictCode_;
};
