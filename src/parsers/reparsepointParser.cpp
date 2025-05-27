#include "reparsepointParser.h"
#include "../utils/stringUtils.h"

template<typename T>
T ReparsePointParser::readLittleEndian(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + sizeof(T) > data.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

std::string ReparsePointParser::readUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t length) {
    if (offset + length > data.size()) {
        return "";
    }
    
    std::wstring wstr;
    for (size_t i = 0; i < length / 2; ++i) {
        uint16_t wchar = readLittleEndian<uint16_t>(data, offset + i * 2);
        if (wchar == 0) break;
        wstr.push_back(static_cast<wchar_t>(wchar));
    }
    
    return StringUtils::wstringToString(wstr);
}

bool ReparsePointParser::parse(const std::vector<uint8_t>& data, size_t offset, ReparsePointAttribute& attr) {
    if (offset + 8 > data.size()) {
        attr.valid = false;
        return false;
    }
    
    try {
        attr.reparseTag = readLittleEndian<uint32_t>(data, offset);
        attr.reparseDataLength = readLittleEndian<uint16_t>(data, offset + 4);
        attr.reserved = readLittleEndian<uint16_t>(data, offset + 6);
        
        if (offset + 8 + attr.reparseDataLength > data.size()) {
            attr.valid = false;
            return false;
        }
        
        attr.reparseData.assign(data.begin() + offset + 8,
                               data.begin() + offset + 8 + attr.reparseDataLength);
        
        attr.type = static_cast<ReparsePointType>(attr.reparseTag);
        
        switch (attr.reparseTag) {
            case SYMLINK:
                parseSymbolicLink(attr.reparseData, attr);
                break;
            case MOUNT_POINT:
                parseMountPoint(attr.reparseData, attr);
                break;
            default:
                break;
        }
        
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

bool ReparsePointParser::parseSymbolicLink(const std::vector<uint8_t>& reparseData, ReparsePointAttribute& attr) {
    if (reparseData.size() < 12) {
        return false;
    }
    
    uint16_t substituteNameOffset = readLittleEndian<uint16_t>(reparseData, 0);
    uint16_t substituteNameLength = readLittleEndian<uint16_t>(reparseData, 2);
    uint16_t printNameOffset = readLittleEndian<uint16_t>(reparseData, 4);
    uint16_t printNameLength = readLittleEndian<uint16_t>(reparseData, 6);
    uint32_t flags = readLittleEndian<uint32_t>(reparseData, 8);
    
    size_t pathDataOffset = 12;
    
    if (pathDataOffset + substituteNameOffset + substituteNameLength <= reparseData.size()) {
        attr.targetPath = readUtf16String(reparseData, 
            pathDataOffset + substituteNameOffset, substituteNameLength);
    }
    
    if (pathDataOffset + printNameOffset + printNameLength <= reparseData.size()) {
        attr.printName = readUtf16String(reparseData, 
            pathDataOffset + printNameOffset, printNameLength);
    }
    
    return true;
}

bool ReparsePointParser::parseMountPoint(const std::vector<uint8_t>& reparseData, ReparsePointAttribute& attr) {
    if (reparseData.size() < 8) {
        return false;
    }
    
    uint16_t substituteNameOffset = readLittleEndian<uint16_t>(reparseData, 0);
    uint16_t substituteNameLength = readLittleEndian<uint16_t>(reparseData, 2);
    uint16_t printNameOffset = readLittleEndian<uint16_t>(reparseData, 4);
    uint16_t printNameLength = readLittleEndian<uint16_t>(reparseData, 6);
    
    size_t pathDataOffset = 8;
    
    if (pathDataOffset + substituteNameOffset + substituteNameLength <= reparseData.size()) {
        attr.targetPath = readUtf16String(reparseData, 
            pathDataOffset + substituteNameOffset, substituteNameLength);
    }
    
    if (pathDataOffset + printNameOffset + printNameLength <= reparseData.size()) {
        attr.printName = readUtf16String(reparseData, 
            pathDataOffset + printNameOffset, printNameLength);
    }
    
    return true;
}

std::string ReparsePointParser::getReparseTypeString(uint32_t reparseTag) {
    switch (reparseTag) {
        case MOUNT_POINT: return "MOUNT_POINT";
        case HSM: return "HSM";
        case DRIVE_EXTENDER: return "DRIVE_EXTENDER";
        case HSM2: return "HSM2";
        case SIS: return "SIS";
        case WIM: return "WIM";
        case CSV: return "CSV";
        case DFS: return "DFS";
        case FILTER_MANAGER: return "FILTER_MANAGER";
        case SYMLINK: return "SYMLINK";
        case IIS_CACHE: return "IIS_CACHE";
        case DFSR: return "DFSR";
        case DEDUP: return "DEDUP";
        case APPXSTRM: return "APPXSTRM";
        case NFS: return "NFS";
        case FILE_PLACEHOLDER: return "FILE_PLACEHOLDER";
        case WOF: return "WOF";
        case WCI: return "WCI";
        case WCI_1: return "WCI_1";
        case GLOBAL_REPARSE: return "GLOBAL_REPARSE";
        case CLOUD: return "CLOUD";
        case APPEXECLINK: return "APPEXECLINK";
        case PROJFS: return "PROJFS";
        case LX_SYMLINK: return "LX_SYMLINK";
        case STORAGE_SYNC: return "STORAGE_SYNC";
        case WCI_TOMBSTONE: return "WCI_TOMBSTONE";
        case UNHANDLED: return "UNHANDLED";
        case ONEDRIVE: return "ONEDRIVE";
        case PROJFS_TOMBSTONE: return "PROJFS_TOMBSTONE";
        case AF_UNIX: return "AF_UNIX";
        default: return "Unknown (0x" + std::to_string(reparseTag) + ")";
    }
}

bool ReparsePointParser::isMicrosoftReparsePoint(uint32_t reparseTag) {
    return (reparseTag & 0x80000000) != 0;
}

bool ReparsePointParser::isNameSurrogate(uint32_t reparseTag) {
    return (reparseTag & 0x20000000) != 0;
}