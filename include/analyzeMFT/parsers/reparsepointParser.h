#ifndef ANALYZEMFT_REPARSEPOINTPARSER_H
#define ANALYZEMFT_REPARSEPOINTPARSER_H

#include <vector>
#include <cstdint>
#include <string>

class ReparsePointParser {
public:
    struct ReparsePointAttribute {
        uint32_t reparseTag;
        uint16_t reparseDataLength;
        uint16_t reserved;
        std::vector<uint8_t> reparseData;
        std::string targetPath;
        std::string printName;
        ReparsePointType type;
        bool valid;
    };

    enum ReparsePointType {
        MOUNT_POINT = 0xA0000003,
        HSM = 0xC0000004,
        DRIVE_EXTENDER = 0x80000005,
        HSM2 = 0x80000006,
        SIS = 0x80000007,
        WIM = 0x80000008,
        CSV = 0x80000009,
        DFS = 0x8000000A,
        FILTER_MANAGER = 0x8000000B,
        SYMLINK = 0xA000000C,
        IIS_CACHE = 0xA0000010,
        DFSR = 0x80000012,
        DEDUP = 0x80000013,
        APPXSTRM = 0xC0000014,
        NFS = 0x80000014,
        FILE_PLACEHOLDER = 0x80000015,
        WOF = 0x80000017,
        WCI = 0x80000018,
        WCI_1 = 0x90001018,
        GLOBAL_REPARSE = 0xA0000019,
        CLOUD = 0x9000001A,
        APPEXECLINK = 0x8000001B,
        PROJFS = 0x9000001C,
        LX_SYMLINK = 0xA000001D,
        STORAGE_SYNC = 0x8000001E,
        WCI_TOMBSTONE = 0xA000001F,
        UNHANDLED = 0x80000020,
        ONEDRIVE = 0x80000021,
        PROJFS_TOMBSTONE = 0xA0000022,
        AF_UNIX = 0x80000023
    };

    static bool parse(const std::vector<uint8_t>& data, size_t offset, ReparsePointAttribute& attr);
    static std::string getReparseTypeString(uint32_t reparseTag);
    static bool isMicrosoftReparsePoint(uint32_t reparseTag);
    static bool isNameSurrogate(uint32_t reparseTag);

private:
    static bool parseSymbolicLink(const std::vector<uint8_t>& reparseData, ReparsePointAttribute& attr);
    static bool parseMountPoint(const std::vector<uint8_t>& reparseData, ReparsePointAttribute& attr);
    static std::string readUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t length);
    
    template<typename T>
    static T readLittleEndian(const std::vector<uint8_t>& data, size_t offset);
};

#endif