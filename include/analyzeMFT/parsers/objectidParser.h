#ifndef ANALYZEMFT_OBJECTIDPARSER_H
#define ANALYZEMFT_OBJECTIDPARSER_H

#include <vector>
#include <cstdint>
#include <string>

class ObjectIdParser {
public:
    struct ObjectIdAttribute {
        std::string objectId;
        std::string birthVolumeId;
        std::string birthObjectId;
        std::string birthDomainId;
        bool valid;
    };

    static bool parse(const std::vector<uint8_t>& data, size_t offset, ObjectIdAttribute& attr);

private:
    static std::string bytesToGuid(const uint8_t* bytes);
    static std::string formatGuid(uint32_t data1, uint16_t data2, uint16_t data3, const uint8_t* data4);
    
    template<typename T>
    static T readLittleEndian(const std::vector<uint8_t>& data, size_t offset);
};

#endif