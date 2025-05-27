#ifndef ANALYZEMFT_ATTRIBUTEPARSER_H
#define ANALYZEMFT_ATTRIBUTEPARSER_H

#include <vector>
#include <cstdint>
#include <string>
#include "../core/constants.h"

class AttributeParser {
public:
    struct AttributeHeader {
        uint32_t type;
        uint32_t length;
        uint8_t nonResident;
        uint8_t nameLength;
        uint16_t nameOffset;
        uint16_t flags;
        uint16_t attributeId;
        uint32_t valueLength;
        uint16_t valueOffset;
        uint8_t indexedFlag;
        uint8_t padding;
    };

    struct NonResidentHeader {
        uint64_t startingVcn;
        uint64_t lastVcn;
        uint16_t dataRunsOffset;
        uint16_t compressionUnit;
        uint32_t padding;
        uint64_t allocatedSize;
        uint64_t actualSize;
        uint64_t initializedSize;
    };

    static bool parseAttributeHeader(const std::vector<uint8_t>& data, size_t offset, AttributeHeader& header);
    static bool parseNonResidentHeader(const std::vector<uint8_t>& data, size_t offset, NonResidentHeader& header);
    static std::string getAttributeName(uint32_t attributeType);
    static std::vector<uint8_t> getAttributeData(const std::vector<uint8_t>& record, size_t offset, const AttributeHeader& header);
    static std::string parseAttributeName(const std::vector<uint8_t>& data, size_t offset, uint8_t nameLength);

private:
    template<typename T>
    static T readLittleEndian(const std::vector<uint8_t>& data, size_t offset);
};

#endif