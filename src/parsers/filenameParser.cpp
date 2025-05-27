#include "filenameParser.h"
#include "../utils/stringUtils.h"

template<typename T>
T FilenameParser::readLittleEndian(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + sizeof(T) > data.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

std::string FilenameParser::readUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t length) {
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

bool FilenameParser::parse(const std::vector<uint8_t>& data, size_t offset, FilenameAttribute& attr) {
    if (offset + 66 > data.size()) {
        attr.valid = false;
        return false;
    }
    
    try {
        attr.parentDirectory = readLittleEndian<uint64_t>(data, offset);
        
        uint32_t crLow = readLittleEndian<uint32_t>(data, offset + 8);
        uint32_t crHigh = readLittleEndian<uint32_t>(data, offset + 12);
        uint32_t mLow = readLittleEndian<uint32_t>(data, offset + 16);
        uint32_t mHigh = readLittleEndian<uint32_t>(data, offset + 20);
        uint32_t aLow = readLittleEndian<uint32_t>(data, offset + 24);
        uint32_t aHigh = readLittleEndian<uint32_t>(data, offset + 28);
        uint32_t eLow = readLittleEndian<uint32_t>(data, offset + 32);
        uint32_t eHigh = readLittleEndian<uint32_t>(data, offset + 36);
        
        attr.creationTime = WindowsTime(crLow, crHigh);
        attr.modificationTime = WindowsTime(mLow, mHigh);
        attr.accessTime = WindowsTime(aLow, aHigh);
        attr.entryTime = WindowsTime(eLow, eHigh);
        
        attr.allocatedSize = readLittleEndian<uint64_t>(data, offset + 40);
        attr.realSize = readLittleEndian<uint64_t>(data, offset + 48);
        attr.flags = readLittleEndian<uint32_t>(data, offset + 56);
        attr.reparseValue = readLittleEndian<uint32_t>(data, offset + 60);
        attr.filenameLength = data[offset + 64];
        attr.filenameNamespace = data[offset + 65];
        
        if (attr.filenameLength > 0) {
            if (offset + 66 + attr.filenameLength * 2 <= data.size()) {
                attr.filename = readUtf16String(data, offset + 66, attr.filenameLength);
            } else {
                attr.valid = false;
                return false;
            }
        }
        
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

std::string FilenameParser::getNamespaceString(uint8_t namespaceValue) {
    switch (namespaceValue) {
        case POSIX: return "POSIX";
        case WIN32: return "WIN32";
        case DOS: return "DOS";
        case WIN32_AND_DOS: return "WIN32_AND_DOS";
        default: return "Unknown (" + std::to_string(namespaceValue) + ")";
    }
}

uint64_t FilenameParser::getParentRecordNumber(uint64_t parentReference) {
    return parentReference & 0x0000FFFFFFFFFFFF;
}

uint16_t FilenameParser::getParentSequenceNumber(uint64_t parentReference) {
    return static_cast<uint16_t>((parentReference >> 48) & 0xFFFF);
}