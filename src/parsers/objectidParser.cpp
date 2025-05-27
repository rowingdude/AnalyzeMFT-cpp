#include "objectidParser.h"
#include <iomanip>
#include <sstream>

template<typename T>
T ObjectIdParser::readLittleEndian(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + sizeof(T) > data.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

bool ObjectIdParser::parse(const std://vector<uint8_t>& data, size_t offset, ObjectIdAttribute& attr) {
    if (offset + 64 > data.size()) {
        attr.valid = false;
        return false;
    }
    
    try {
        attr.objectId = bytesToGuid(&data[offset]);
        attr.birthVolumeId = bytesToGuid(&data[offset + 16]);
        attr.birthObjectId = bytesToGuid(&data[offset + 32]);
        attr.birthDomainId = bytesToGuid(&data[offset + 48]);
        
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

std::string ObjectIdParser::bytesToGuid(const uint8_t* bytes) {
    uint32_t data1 = readLittleEndian<uint32_t>(std::vector<uint8_t>(bytes, bytes + 16), 0);
    uint16_t data2 = readLittleEndian<uint16_t>(std::vector<uint8_t>(bytes, bytes + 16), 4);
    uint16_t data3 = readLittleEndian<uint16_t>(std::vector<uint8_t>(bytes, bytes + 16), 6);
    
    return formatGuid(data1, data2, data3, bytes + 8);
}

std::string ObjectIdParser::formatGuid(uint32_t data1, uint16_t data2, uint16_t data3, const uint8_t* data4) {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    
    oss << std::setw(8) << data1 << "-";
    oss << std::setw(4) << data2 << "-";
    oss << std::setw(4) << data3 << "-";
    oss << std::setw(2) << static_cast<int>(data4[0]);
    oss << std::setw(2) << static_cast<int>(data4[1]) << "-";
    
    for (int i = 2; i < 8; ++i) {
        oss << std::setw(2) << static_cast<int>(data4[i]);
    }
    
    return oss.str();
}