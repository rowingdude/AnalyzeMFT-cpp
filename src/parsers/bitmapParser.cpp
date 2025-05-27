#include "bitmapParser.h"

template<typename T>
T BitmapParser::readLittleEndian(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + sizeof(T) > data.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

bool BitmapParser::parse(const std::vector<uint8_t>& data, size_t offset, BitmapAttribute& attr) {
    if (data.empty()) {
        attr.valid = false;
        return false;
    }
    
    try {
        attr.bitmap = data;
        attr.totalBits = data.size() * 8;
        attr.setBits = countSetBits(data);
        attr.clearBits = attr.totalBits - attr.setBits;
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

bool BitmapParser::isBitSet(const std::vector<uint8_t>& bitmap, uint64_t bitIndex) {
    uint64_t byteIndex = bitIndex / 8;
    uint8_t bitPosition = bitIndex % 8;
    
    if (byteIndex >= bitmap.size()) {
        return false;
    }
    
    return (bitmap[byteIndex] & (1 << bitPosition)) != 0;
}

void BitmapParser::setBit(std::vector<uint8_t>& bitmap, uint64_t bitIndex) {
    uint64_t byteIndex = bitIndex / 8;
    uint8_t bitPosition = bitIndex % 8;
    
    if (byteIndex < bitmap.size()) {
        bitmap[byteIndex] |= (1 << bitPosition);
    }
}

void BitmapParser::clearBit(std::vector<uint8_t>& bitmap, uint64_t bitIndex) {
    uint64_t byteIndex = bitIndex / 8;
    uint8_t bitPosition = bitIndex % 8;
    
    if (byteIndex < bitmap.size()) {
        bitmap[byteIndex] &= ~(1 << bitPosition);
    }
}

uint64_t BitmapParser::countSetBits(const std::vector<uint8_t>& bitmap) {
    uint64_t count = 0;
    
    for (uint8_t byte : bitmap) {
        count += popcount(byte);
    }
    
    return count;
}

uint64_t BitmapParser::popcount(uint64_t value) {
#ifdef __GNUC__
    return __builtin_popcountll(value);
#else
    uint64_t count = 0;
    while (value) {
        count += value & 1;
        value >>= 1;
    }
    return count;
#endif
}

std::string BitmapParser::getBitmapSummary(const BitmapAttribute& attr) {
    if (!attr.valid) {
        return "Invalid bitmap";
    }
    
    double usagePercent = attr.totalBits > 0 ? 
        (static_cast<double>(attr.setBits) / attr.totalBits) * 100.0 : 0.0;
    
    return "Total bits: " + std::to_string(attr.totalBits) +
           ", Set: " + std::to_string(attr.setBits) +
           ", Clear: " + std::to_string(attr.clearBits) +
           ", Usage: " + std::to_string(usagePercent) + "%";
}