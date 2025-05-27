#ifndef ANALYZEMFT_HASHCALC_H
#define ANALYZEMFT_HASHCALC_H

#include <string>
#include <vector>
#include <cstdint>

#ifdef CRC32_HARDWARE
#include <immintrin.h>
#endif

class HashCalculator {
public:
    HashCalculator();
    ~HashCalculator();
    
    std::string calculateMd5(const std::vector<uint8_t>& data);
    std::string calculateSha256(const std::vector<uint8_t>& data);
    std::string calculateSha512(const std::vector<uint8_t>& data);
    std::string calculateCrc32(const std::vector<uint8_t>& data);
    
private:
    std::string bytesToHex(const std::vector<uint8_t>& bytes);
    uint32_t crc32Software(const std::vector<uint8_t>& data);
    uint32_t crc32Hardware(const std::vector<uint8_t>& data);
    
    static const uint32_t CRC32_TABLE[256];
};

#endif