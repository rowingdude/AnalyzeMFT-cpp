#include "volumeParser.h"
#include "../utils/stringUtils.h"

template<typename T>
T VolumeParser::readLittleEndian(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + sizeof(T) > data.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

std::string VolumeParser::readUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t length) {
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

bool VolumeParser::parseVolumeName(const std::vector<uint8_t>& data, size_t offset, VolumeNameAttribute& attr) {
    if (data.empty()) {
        attr.valid = false;
        return false;
    }
    
    try {
        size_t nameLength = data.size() / 2;
        attr.volumeName = readUtf16String(data, 0, nameLength);
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

bool VolumeParser::parseVolumeInformation(const std::vector<uint8_t>& data, size_t offset, VolumeInformationAttribute& attr) {
    if (offset + 12 > data.size()) {
        attr.valid = false;
        return false;
    }
    
    try {
        attr.reserved1 = readLittleEndian<uint64_t>(data, offset);
        attr.majorVersion = data[offset + 8];
        attr.minorVersion = data[offset + 9];
        attr.flags = readLittleEndian<uint16_t>(data, offset + 10);
        
        if (offset + 16 <= data.size()) {
            attr.reserved2 = readLittleEndian<uint32_t>(data, offset + 12);
        } else {
            attr.reserved2 = 0;
        }
        
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

std::string VolumeParser::getVolumeFlagsString(uint16_t flags) {
    std::vector<std::string> flagStrings;
    
    if (flags & VOLUME_IS_DIRTY) flagStrings.push_back("DIRTY");
    if (flags & VOLUME_RESIZE_LOG_FILE) flagStrings.push_back("RESIZE_LOG_FILE");
    if (flags & VOLUME_UPGRADE_ON_MOUNT) flagStrings.push_back("UPGRADE_ON_MOUNT");
    if (flags & VOLUME_MOUNTED_ON_NT4) flagStrings.push_back("MOUNTED_ON_NT4");
    if (flags & VOLUME_DELETE_USN_UNDERWAY) flagStrings.push_back("DELETE_USN_UNDERWAY");
    if (flags & VOLUME_REPAIR_OBJECT_ID) flagStrings.push_back("REPAIR_OBJECT_ID");
    if (flags & VOLUME_CHKDSK_UNDERWAY) flagStrings.push_back("CHKDSK_UNDERWAY");
    if (flags & VOLUME_MODIFIED_BY_CHKDSK) flagStrings.push_back("MODIFIED_BY_CHKDSK");
    
    if (flagStrings.empty()) {
        return "NONE";
    }
    
    std::string result = flagStrings[0];
    for (size_t i = 1; i < flagStrings.size(); ++i) {
        result += " | " + flagStrings[i];
    }
    return result;
}