#include "xattrParser.h"

template<typename T>
T ExtendedAttributeParser::readLittleEndian(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + sizeof(T) > data.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

std::string ExtendedAttributeParser::readAsciiString(const std::vector<uint8_t>& data, size_t offset, size_t length) {
    if (offset + length > data.size()) {
        return "";
    }
    
    std::string result;
    for (size_t i = 0; i < length; ++i) {
        char c = static_cast<char>(data[offset + i]);
        if (c == 0) break;
        result += c;
    }
    return result;
}

bool ExtendedAttributeParser::parseEaInformation(const std::vector<uint8_t>& data, size_t offset, EaInformationAttribute& attr) {
    if (offset + 8 > data.size()) {
        attr.valid = false;
        return false;
    }
    
    try {
        attr.packedEaSize = readLittleEndian<uint32_t>(data, offset);
        attr.needEaCount = readLittleEndian<uint32_t>(data, offset + 4);
        
        if (offset + 12 <= data.size()) {
            attr.unpackedEaSize = readLittleEndian<uint32_t>(data, offset + 8);
        } else {
            attr.unpackedEaSize = attr.packedEaSize;
        }
        
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

bool ExtendedAttributeParser::parseEa(const std::vector<uint8_t>& data, size_t offset, EaAttribute& attr) {
    if (data.empty()) {
        attr.valid = false;
        return false;
    }
    
    try {
        attr.attributes.clear();
        attr.totalSize = static_cast<uint32_t>(data.size());
        
        size_t currentOffset = 0;
        
        while (currentOffset + 8 <= data.size()) {
            ExtendedAttribute ea;
            
            ea.nextEntryOffset = readLittleEndian<uint32_t>(data, currentOffset);
            ea.flags = data[currentOffset + 4];
            ea.nameLength = data[currentOffset + 5];
            ea.valueLength = readLittleEndian<uint16_t>(data, currentOffset + 6);
            
            ea.needEa = (ea.flags & EA_NEED_EA) != 0;
            
            if (currentOffset + 8 + ea.nameLength > data.size()) {
                break;
            }
            
            ea.name = readAsciiString(data, currentOffset + 8, ea.nameLength);
            
            size_t valueOffset = currentOffset + 8 + ea.nameLength;
            if (ea.nameLength % 2 == 0) {
                valueOffset++;
            }
            
            if (valueOffset + ea.valueLength <= data.size()) {
                ea.value.assign(data.begin() + valueOffset,
                               data.begin() + valueOffset + ea.valueLength);
            }
            
            attr.attributes.push_back(ea);
            
            if (ea.nextEntryOffset == 0) {
                break;
            }
            
            currentOffset += ea.nextEntryOffset;
        }
        
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

bool ExtendedAttributeParser::parseLoggedUtilityStream(const std::vector<uint8_t>& data, size_t offset, LoggedUtilityStreamAttribute& attr) {
    if (data.empty()) {
        attr.valid = false;
        return false;
    }
    
    try {
        attr.streamSize = data.size();
        attr.streamData = data;
        attr.streamName = "LOGGED_UTILITY_STREAM";
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

std::string ExtendedAttributeParser::getEaFlagsString(uint8_t flags) {
    std::vector<std::string> flagStrings;
    
    if (flags & EA_NEED_EA) {
        flagStrings.push_back("NEED_EA");
    }
    
    if (flagStrings.empty()) {
        return "NONE";
    }
    
    std::string result = flagStrings[0];
    for (size_t i = 1; i < flagStrings.size(); ++i) {
        result += " | " + flagStrings[i];
    }
    return result;
}