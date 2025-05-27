#ifndef ANALYZEMFT_SECDESCPARSER_H
#define ANALYZEMFT_SECDESCPARSER_H

#include <vector>
#include <cstdint>
#include <string>

class SecurityDescriptorParser {
public:
    struct SecurityDescriptor {
        uint8_t revision;
        uint8_t padding1;
        uint16_t control;
        uint32_t ownerOffset;
        uint32_t groupOffset;
        uint32_t saclOffset;
        uint32_t daclOffset;
        std::string ownerSid;
        std::string groupSid;
        std::vector<std::string> dacl;
        std::vector<std::string> sacl;
        bool valid;
    };

    struct AccessControlEntry {
        uint8_t aceType;
        uint8_t aceFlags;
        uint16_t aceSize;
        uint32_t accessMask;
        std::string sid;
    };

    static bool parse(const std::vector<uint8_t>& data, size_t offset, SecurityDescriptor& desc);
    static std::string parseSid(const std::vector<uint8_t>& data, size_t offset);
    static std::vector<AccessControlEntry> parseAcl(const std::vector<uint8_t>& data, size_t offset);
    static std::string getAceTypeString(uint8_t aceType);
    static std::string getControlFlagsString(uint16_t control);

private:
    static const uint8_t ACCESS_ALLOWED_ACE_TYPE = 0x00;
    static const uint8_t ACCESS_DENIED_ACE_TYPE = 0x01;
    static const uint8_t SYSTEM_AUDIT_ACE_TYPE = 0x02;
    static const uint8_t SYSTEM_ALARM_ACE_TYPE = 0x03;
    
    static const uint16_t SE_OWNER_DEFAULTED = 0x0001;
    static const uint16_t SE_GROUP_DEFAULTED = 0x0002;
    static const uint16_t SE_DACL_PRESENT = 0x0004;
    static const uint16_t SE_DACL_DEFAULTED = 0x0008;
    static const uint16_t SE_SACL_PRESENT = 0x0010;
    static const uint16_t SE_SACL_DEFAULTED = 0x0020;
    static const uint16_t SE_DACL_AUTO_INHERIT_REQ = 0x0100;
    static const uint16_t SE_SACL_AUTO_INHERIT_REQ = 0x0200;
    static const uint16_t SE_DACL_AUTO_INHERITED = 0x0400;
    static const uint16_t SE_SACL_AUTO_INHERITED = 0x0800;
    static const uint16_t SE_DACL_PROTECTED = 0x1000;
    static const uint16_t SE_SACL_PROTECTED = 0x2000;
    static const uint16_t SE_RM_CONTROL_VALID = 0x4000;
    static const uint16_t SE_SELF_RELATIVE = 0x8000;

    template<typename T>
    static T readLittleEndian(const std::vector<uint8_t>& data, size_t offset);
};

#endif