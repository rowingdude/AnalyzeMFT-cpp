#include "secdescParser.h"
#include <sstream>
#include <iomanip>

template<typename T>
T SecurityDescriptorParser::readLittleEndian(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + sizeof(T) > data.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

bool SecurityDescriptorParser::parse(const std::vector<uint8_t>& data, size_t offset, SecurityDescriptor& desc) {
    if (offset + 20 > data.size()) {
        desc.valid = false;
        return false;
    }
    
    try {
        desc.revision = data[offset];
        desc.padding1 = data[offset + 1];
        desc.control = readLittleEndian<uint16_t>(data, offset + 2);
        desc.ownerOffset = readLittleEndian<uint32_t>(data, offset + 4);
        desc.groupOffset = readLittleEndian<uint32_t>(data, offset + 8);
        desc.saclOffset = readLittleEndian<uint32_t>(data, offset + 12);
        desc.daclOffset = readLittleEndian<uint32_t>(data, offset + 16);
        
        if (desc.ownerOffset > 0 && offset + desc.ownerOffset < data.size()) {
            desc.ownerSid = parseSid(data, offset + desc.ownerOffset);
        }
        
        if (desc.groupOffset > 0 && offset + desc.groupOffset < data.size()) {
            desc.groupSid = parseSid(data, offset + desc.groupOffset);
        }
        
        if (desc.daclOffset > 0 && offset + desc.daclOffset < data.size()) {
            std::vector<AccessControlEntry> dacl = parseAcl(data, offset + desc.daclOffset);
            for (const auto& ace : dacl) {
                desc.dacl.push_back(ace.sid + " (" + getAceTypeString(ace.aceType) + ")");
            }
        }
        
        if (desc.saclOffset > 0 && offset + desc.saclOffset < data.size()) {
            std::vector<AccessControlEntry> sacl = parseAcl(data, offset + desc.saclOffset);
            for (const auto& ace : sacl) {
                desc.sacl.push_back(ace.sid + " (" + getAceTypeString(ace.aceType) + ")");
            }
        }
        
        desc.valid = true;
        return true;
    } catch (const std::exception& e) {
        desc.valid = false;
        return false;
    }
}

std::string SecurityDescriptorParser::parseSid(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + 8 > data.size()) {
        return "";
    }
    
    uint8_t revision = data[offset];
    uint8_t subAuthorityCount = data[offset + 1];
    
    if (offset + 8 + subAuthorityCount * 4 > data.size()) {
        return "";
    }
    
    uint64_t identifierAuthority = 0;
    for (int i = 0; i < 6; ++i) {
        identifierAuthority = (identifierAuthority << 8) | data[offset + 2 + i];
    }
    
    std::ostringstream oss;
    oss << "S-" << static_cast<int>(revision) << "-" << identifierAuthority;
    
    for (uint8_t i = 0; i < subAuthorityCount; ++i) {
        uint32_t subAuthority = readLittleEndian<uint32_t>(data, offset + 8 + i * 4);
        oss << "-" << subAuthority;
    }
    
    return oss.str();
}

std::vector<SecurityDescriptorParser::AccessControlEntry> SecurityDescriptorParser::parseAcl(const std::vector<uint8_t>& data, size_t offset) {
    std::vector<AccessControlEntry> entries;
    
    if (offset + 8 > data.size()) {
        return entries;
    }
    
    uint8_t aclRevision = data[offset];
    uint8_t padding1 = data[offset + 1];
    uint16_t aclSize = readLittleEndian<uint16_t>(data, offset + 2);
    uint16_t aceCount = readLittleEndian<uint16_t>(data, offset + 4);
    uint16_t padding2 = readLittleEndian<uint16_t>(data, offset + 6);
    
    size_t aceOffset = offset + 8;
    
    for (uint16_t i = 0; i < aceCount && aceOffset < data.size(); ++i) {
        if (aceOffset + 8 > data.size()) {
            break;
        }
        
        AccessControlEntry ace;
        ace.aceType = data[aceOffset];
        ace.aceFlags = data[aceOffset + 1];
        ace.aceSize = readLittleEndian<uint16_t>(data, aceOffset + 2);
        ace.accessMask = readLittleEndian<uint32_t>(data, aceOffset + 4);
        
        if (aceOffset + ace.aceSize <= data.size()) {
            ace.sid = parseSid(data, aceOffset + 8);
        }
        
        entries.push_back(ace);
        aceOffset += ace.aceSize;
    }
    
    return entries;
}

std::string SecurityDescriptorParser::getAceTypeString(uint8_t aceType) {
    switch (aceType) {
        case ACCESS_ALLOWED_ACE_TYPE: return "ACCESS_ALLOWED";
        case ACCESS_DENIED_ACE_TYPE: return "ACCESS_DENIED";
        case SYSTEM_AUDIT_ACE_TYPE: return "SYSTEM_AUDIT";
        case SYSTEM_ALARM_ACE_TYPE: return "SYSTEM_ALARM";
        default: return "Unknown (" + std::to_string(aceType) + ")";
    }
}

std::string SecurityDescriptorParser::getControlFlagsString(uint16_t control) {
    std::vector<std::string> flags;
    
    if (control & SE_OWNER_DEFAULTED) flags.push_back("OWNER_DEFAULTED");
    if (control & SE_GROUP_DEFAULTED) flags.push_back("GROUP_DEFAULTED");
    if (control & SE_DACL_PRESENT) flags.push_back("DACL_PRESENT");
    if (control & SE_DACL_DEFAULTED) flags.push_back("DACL_DEFAULTED");
    if (control & SE_SACL_PRESENT) flags.push_back("SACL_PRESENT");
    if (control & SE_SACL_DEFAULTED) flags.push_back("SACL_DEFAULTED");
    if (control & SE_DACL_AUTO_INHERIT_REQ) flags.push_back("DACL_AUTO_INHERIT_REQ");
    if (control & SE_SACL_AUTO_INHERIT_REQ) flags.push_back("SACL_AUTO_INHERIT_REQ");
    if (control & SE_DACL_AUTO_INHERITED) flags.push_back("DACL_AUTO_INHERITED");
    if (control & SE_SACL_AUTO_INHERITED) flags.push_back("SACL_AUTO_INHERITED");
    if (control & SE_DACL_PROTECTED) flags.push_back("DACL_PROTECTED");
    if (control & SE_SACL_PROTECTED) flags.push_back("SACL_PROTECTED");
    if (control & SE_RM_CONTROL_VALID) flags.push_back("RM_CONTROL_VALID");
    if (control & SE_SELF_RELATIVE) flags.push_back("SELF_RELATIVE");
    
    if (flags.empty()) {
        return "NONE";
    }
    
    std::string result = flags[0];
    for (size_t i = 1; i < flags.size(); ++i) {
        result += " | " + flags[i];
    }
    return result;
}