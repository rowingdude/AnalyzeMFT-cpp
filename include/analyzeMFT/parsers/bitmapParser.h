#ifndef ANALYZEMFT_BITMAPPARSER_H
#define ANALYZEMFT_BITMAPPARSER_H

#include <vector>
#include <cstdint>
#include <string>

class BitmapParser {
public:
    struct BitmapAttribute {
        std::vector<uint8_t> bitmap;
        uint64_t totalBits;
        uint64_t setBits;
        uint64_t clearBits;
        bool valid;
    };

    static bool parse(const std::vector<uint8_t>& data, size_t offset, BitmapAttribute& attr);
    static bool isBitSet(const std::vector<uint8_t>& bitmap, uint64_t bitIndex);
    static void setBit(std::vector<uint8_t>& bitmap, uint64_t bitIndex);
    static void clearBit(std::vector<uint8_t>& bitmap, uint64_t bitIndex);
    static uint64_t countSetBits(const std::vector<uint8_t>& bitmap);
    static std::string getBitmapSummary(const BitmapAttribute& attr);

private:
    static uint64_t popcount(uint64_t value);
    
    template<typename T>
    static T readLittleEndian(const std::vector<uint8_t>& data, size_t offset);
};

#endif