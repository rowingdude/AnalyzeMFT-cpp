#include "attributeParser.h"
#include "../utils/stringUtils.h"

template<typename T>
T AttributeParser::readLittleEndian(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + sizeof(T) > data.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

bool AttributeParser::parseAttributeHeader(const std::vector<uint8_t>& data, size_t offset, AttributeHeader& header) {
    if (offset + 16 > data.size()) {
        return false;
    }
    
    header.type = readLittleEndian<uint32_t>(data, offset);
    header.length = readLittleEndian<uint32_t>(data, offset + 4);
    header.nonResident = data[offset + 8];
    header.nameLength = data[offset + 9];
    header.nameOffset = readLittleEndian<uint16_t>(data, offset + 10);
    header.flags = readLittleEndian<uint16_t>(data, offset + 12);
    header.attributeId = readLittleEndian<uint16_t>(data, offset + 14);
    
    if (header.nonResident == 0) {
        if (offset + 24 > data.size()) {
            return false;
        }
        header.valueLength = readLittleEndian<uint32_t>(data, offset + 16);
        header.valueOffset = readLittleEndian<uint16_t>(data, offset + 20);
        header.indexedFlag = data[offset + 22];
        header.padding = data[offset + 23];
    }
    
    return true;
}

bool AttributeParser::parseNonResidentHeader(const std::vector<uint8_t>& data, size_t offset, NonResidentHeader& header) {
    if (offset + 48 > data.size()) {
        return false;
    }
    
    header.startingVcn = readLittleEndian<uint64_t>(data, offset + 16);
    header.lastVcn = readLittleEndian<uint64_t>(data, offset + 24);
    header.dataRunsOffset = readLittleEndian<uint16_t>(data, offset + 32);
    header.compressionUnit = readLittleEndian<uint16_t>(data, offset + 34);
    header.padding = readLittleEndian<uint32_t>(data, offset + 36);
    header.allocatedSize = readLittleEndian<uint64_t>(data, offset + 40);
    header.actualSize = readLittleEndian<uint64_t>(data, offset + 48);
    header.initializedSize = readLittleEndian<uint64_t>(data, offset + 56);
    
    return true;
}

std::string AttributeParser::getAttributeName(uint32_t attributeType) {
    auto it = ATTRIBUTE_NAMES.find(attributeType);
    if (it != ATTRIBUTE_NAMES.end()) {
        return it->second;
    }
    return "Unknown (" + std::to_string(attributeType) + ")";
}

std::vector<uint8_t> AttributeParser::getAttributeData(const std::vector<uint8_t>& record, size_t offset, const AttributeHeader& header) {
    if (header.nonResident == 0) {
        size_t startOffset = offset + header.valueOffset;
        size_t endOffset = startOffset + header.valueLength;
        
        if (endOffset <= record.size()) {
            return std::vector<uint8_t>(record.begin() + startOffset, record.begin() + endOffset);
        }
    }
    return {};
}

std::string AttributeParser::parseAttributeName(const std::vector<uint8_t>& data, size_t offset, uint8_t nameLength) {
    if (nameLength == 0 || offset + nameLength * 2 > data.size()) {
        return "";
    }
    
    std::wstring wstr;
    for (uint8_t i = 0; i < nameLength; ++i) {
        uint16_t wchar = readLittleEndian<uint16_t>(data, offset + i * 2);
        wstr.push_back(static_cast<wchar_t>(wchar));
    }
    
    return StringUtils::wstringToString(wstr);
}