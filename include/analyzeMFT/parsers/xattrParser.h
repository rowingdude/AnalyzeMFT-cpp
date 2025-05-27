#ifndef ANALYZEMFT_XATTRPARSER_H
#define ANALYZEMFT_XATTRPARSER_H

#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>

class ExtendedAttributeParser {
public:
    struct EaInformationAttribute {
        uint32_t packedEaSize;
        uint32_t needEaCount;
        uint32_t unpackedEaSize;
        bool valid;
    };

    struct ExtendedAttribute {
        uint32_t nextEntryOffset;
        uint8_t flags;
        uint8_t nameLength;
        uint16_t valueLength;
        std::string name;
        std::vector<uint8_t> value;
        bool needEa;
    };

    struct EaAttribute {
        std::vector<ExtendedAttribute> attributes;
        uint32_t totalSize;
        bool valid;
    };

    struct LoggedUtilityStreamAttribute {
        uint64_t streamSize;
        std::vector<uint8_t> streamData;
        std::string streamName;
        bool valid;
    };

    static bool parseEaInformation(const std::vector<uint8_t>& data, size_t offset, EaInformationAttribute& attr);
    static bool parseEa(const std::vector<uint8_t>& data, size_t offset, EaAttribute& attr);
    static bool parseLoggedUtilityStream(const std::vector<uint8_t>& data, size_t offset, LoggedUtilityStreamAttribute& attr);
    static std::string getEaFlagsString(uint8_t flags);

private:
    static const uint8_t EA_NEED_EA = 0x80;
    
    template<typename T>
    static T readLittleEndian(const std::vector<uint8_t>& data, size_t offset);
    
    static std::string readAsciiString(const std::vector<uint8_t>& data, size_t offset, size_t length);
};

#endif