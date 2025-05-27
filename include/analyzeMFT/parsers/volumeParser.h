#ifndef ANALYZEMFT_VOLUMEPARSER_H
#define ANALYZEMFT_VOLUMEPARSER_H

#include <vector>
#include <cstdint>
#include <string>

class VolumeParser {
public:
    struct VolumeNameAttribute {
        std::string volumeName;
        bool valid;
    };

    struct VolumeInformationAttribute {
        uint64_t reserved1;
        uint8_t majorVersion;
        uint8_t minorVersion;
        uint16_t flags;
        uint32_t reserved2;
        bool valid;
    };

    static bool parseVolumeName(const std::vector<uint8_t>& data, size_t offset, VolumeNameAttribute& attr);
    static bool parseVolumeInformation(const std::vector<uint8_t>& data, size_t offset, VolumeInformationAttribute& attr);
    static std::string getVolumeFlagsString(uint16_t flags);

private:
    static const uint16_t VOLUME_IS_DIRTY = 0x0001;
    static const uint16_t VOLUME_RESIZE_LOG_FILE = 0x0002;
    static const uint16_t VOLUME_UPGRADE_ON_MOUNT = 0x0004;
    static const uint16_t VOLUME_MOUNTED_ON_NT4 = 0x0008;
    static const uint16_t VOLUME_DELETE_USN_UNDERWAY = 0x0010;
    static const uint16_t VOLUME_REPAIR_OBJECT_ID = 0x0020;
    static const uint16_t VOLUME_CHKDSK_UNDERWAY = 0x4000;
    static const uint16_t VOLUME_MODIFIED_BY_CHKDSK = 0x8000;

    template<typename T>
    static T readLittleEndian(const std::vector<uint8_t>& data, size_t offset);
    
    static std::string readUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t length);
};

#endif