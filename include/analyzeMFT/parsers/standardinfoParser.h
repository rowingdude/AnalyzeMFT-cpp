#ifndef ANALYZEMFT_STANDARDINFOPARSER_H
#define ANALYZEMFT_STANDARDINFOPARSER_H

#include <vector>
#include <cstdint>
#include "../core/winTime.h"

class StandardInfoParser {
public:
    struct StandardInformation {
        WindowsTime creationTime;
        WindowsTime modificationTime;
        WindowsTime accessTime;
        WindowsTime entryTime;
        uint32_t fileAttributes;
        uint32_t maxVersions;
        uint32_t versionNumber;
        uint32_t classId;
        uint32_t ownerId;
        uint32_t securityId;
        uint64_t quotaCharged;
        uint64_t updateSequenceNumber;
        bool valid;
    };

    static bool parse(const std::vector<uint8_t>& data, size_t offset, StandardInformation& info);
    static std::string getFileAttributesString(uint32_t attributes);

private:
    static const uint32_t FILE_ATTRIBUTE_READONLY = 0x00000001;
    static const uint32_t FILE_ATTRIBUTE_HIDDEN = 0x00000002;
    static const uint32_t FILE_ATTRIBUTE_SYSTEM = 0x00000004;
    static const uint32_t FILE_ATTRIBUTE_DIRECTORY = 0x00000010;
    static const uint32_t FILE_ATTRIBUTE_ARCHIVE = 0x00000020;
    static const uint32_t FILE_ATTRIBUTE_DEVICE = 0x00000040;
    static const uint32_t FILE_ATTRIBUTE_NORMAL = 0x00000080;
    static const uint32_t FILE_ATTRIBUTE_TEMPORARY = 0x00000100;
    static const uint32_t FILE_ATTRIBUTE_SPARSE_FILE = 0x00000200;
    static const uint32_t FILE_ATTRIBUTE_REPARSE_POINT = 0x00000400;
    static const uint32_t FILE_ATTRIBUTE_COMPRESSED = 0x00000800;
    static const uint32_t FILE_ATTRIBUTE_OFFLINE = 0x00001000;
    static const uint32_t FILE_ATTRIBUTE_NOT_CONTENT_INDEXED = 0x00002000;
    static const uint32_t FILE_ATTRIBUTE_ENCRYPTED = 0x00004000;

    template<typename T>
    static T readLittleEndian(const std::vector<uint8_t>& data, size_t offset);
};

#endif