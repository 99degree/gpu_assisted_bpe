#pragma once
#include <vector>
#include <string>
#include <cstdint>

class ByteAligner {
public:
    static std::vector<uint32_t> alignBytes(const std::vector<uint8_t>& bytes);
    static std::vector<uint32_t> alignString(const std::string& str);
};
