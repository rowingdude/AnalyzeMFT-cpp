#include "dataParser.h"
#include "../utils/stringUtils.h"

template<typename T>
T DataParser::readLittleEndian(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + sizeof(T) > data.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

std::string DataParser::readUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t length) {
    if (offset + length * 2 > data.size()) {
        return "";
    }
    
    std::wstring wstr;
    for (size_t i = 0; i < length; ++i) {
        uint16_t wchar = readLittleEndian<uint16_t>(data, offset + i * 2);
        wstr.push_back(static_cast<wchar_t>(wchar));
    }
    
    return StringUtils::wstringToString(wstr);
}

bool DataParser::parse(const std::vector<uint8_t>& data, size_t offset, DataAttribute& attr) {
    if (offset + 24 > data.size()) {
        attr.valid = false;
        return false;
    }
    
    try {
        uint8_t nonResidentFlag = data[offset + 8];
        uint8_t nameLength = data[offset + 9];
        uint16_t nameOffset = readLittleEndian<uint16_t>(data, offset + 10);
        
        if (nameLength > 0 && offset + nameOffset + nameLength * 2 <= data.size()) {
            attr.name = readUtf16String(data, offset + nameOffset, nameLength);
        }
        
        attr.nonResident = (nonResidentFlag != 0);
        
        if (!attr.nonResident) {
            attr.contentSize = readLittleEndian<uint32_t>(data, offset + 16);
            uint16_t contentOffset = readLittleEndian<uint16_t>(data, offset + 20);
            
            if (offset + contentOffset + attr.contentSize <= data.size()) {
                attr.content.assign(data.begin() + offset + contentOffset,
                                   data.begin() + offset + contentOffset + attr.contentSize);
            }
        } else {
            if (offset + 64 <= data.size()) {
                attr.startingVcn = readLittleEndian<uint64_t>(data, offset + 16);
                attr.lastVcn = readLittleEndian<uint64_t>(data, offset + 24);
                attr.dataRunsOffset = readLittleEndian<uint16_t>(data, offset + 32);
                attr.compressionUnit = readLittleEndian<uint16_t>(data, offset + 34);
                attr.allocatedSize = readLittleEndian<uint64_t>(data, offset + 40);
                attr.actualSize = readLittleEndian<uint64_t>(data, offset + 48);
                attr.initializedSize = readLittleEndian<uint64_t>(data, offset + 56);
                
                if (attr.dataRunsOffset > 0) {
                    attr.dataRuns = parseDataRuns(data, offset + attr.dataRunsOffset);
                }
            }
        }
        
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

std::vector<DataParser::DataRun> DataParser::parseDataRuns(const std::vector<uint8_t>& data, size_t offset) {
    std::vector<DataRun> runs;
    
    while (offset < data.size()) {
        uint8_t header = data[offset];
        if (header == 0) {
            break;
        }
        
        uint8_t lengthBytes = header & 0x0F;
        uint8_t offsetBytes = (header >> 4) & 0x0F;
        
        if (lengthBytes == 0 || offset + 1 + lengthBytes + offsetBytes > data.size()) {
            break;
        }
        
        DataRun run;
        run.length = readUnsignedInteger(data, offset + 1, lengthBytes);
        
        if (offsetBytes > 0) {
            run.offset = readSignedInteger(data, offset + 1 + lengthBytes, offsetBytes);
            run.sparse = false;
        } else {
            run.offset = 0;
            run.sparse = true;
        }
        
        runs.push_back(run);
        offset += 1 + lengthBytes + offsetBytes;
    }
    
    return runs;
}

uint64_t DataParser::calculateTotalClusters(const std::vector<DataRun>& dataRuns) {
    uint64_t total = 0;
    for (const auto& run : dataRuns) {
        total += run.length;
    }
    return total;
}

int64_t DataParser::readSignedInteger(const std::vector<uint8_t>& data, size_t offset, size_t length) {
    if (length == 0 || length > 8 || offset + length > data.size()) {
        return 0;
    }
    
    int64_t value = 0;
    bool negative = (data[offset + length - 1] & 0x80) != 0;
    
    for (size_t i = 0; i < length; ++i) {
        value |= static_cast<int64_t>(data[offset + i]) << (i * 8);
    }
    
    if (negative) {
        int64_t mask = -1LL << (length * 8);
        value |= mask;
    }
    
    return value;
}

uint64_t DataParser::readUnsignedInteger(const std::vector<uint8_t>& data, size_t offset, size_t length) {
    if (length == 0 || length > 8 || offset + length > data.size()) {
        return 0;
    }
    
    uint64_t value = 0;
    for (size_t i = 0; i < length; ++i) {
        value |= static_cast<uint64_t>(data[offset + i]) << (i * 8);
    }
    
    return value;
}