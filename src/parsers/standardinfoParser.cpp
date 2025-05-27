#include "standardinfoParser.h"

template<typename T>
T StandardInfoParser::readLittleEndian(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + sizeof(T) > data.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

bool StandardInfoParser::parse(const std::vector<uint8_t>& data, size_t offset, StandardInformation& info) {
    if (offset + 48 > data.size()) {
        info.valid = false;
        return false;
    }
    
    try {
        uint32_t crLow = readLittleEndian<uint32_t>(data, offset);
        uint32_t crHigh = readLittleEndian<uint32_t>(data, offset + 4);
        uint32_t mLow = readLittleEndian<uint32_t>(data, offset + 8);
        uint32_t mHigh = readLittleEndian<uint32_t>(data, offset + 12);
        uint32_t aLow = readLittleEndian<uint32_t>(data, offset + 16);
        uint32_t aHigh = readLittleEndian<uint32_t>(data, offset + 20);
        uint32_t eLow = readLittleEndian<uint32_t>(data, offset + 24);
        uint32_t eHigh = readLittleEndian<uint32_t>(data, offset + 28);
        
        info.creationTime = WindowsTime(crLow, crHigh);
        info.modificationTime = WindowsTime(mLow, mHigh);
        info.accessTime = WindowsTime(aLow, aHigh);
        info.entryTime = WindowsTime(eLow, eHigh);
        
        info.fileAttributes = readLittleEndian<uint32_t>(data, offset + 32);
        
        if (offset + 72 <= data.size()) {
            info.maxVersions = readLittleEndian<uint32_t>(data, offset + 36);
            info.versionNumber = readLittleEndian<uint32_t>(data, offset + 40);
            info.classId = readLittleEndian<uint32_t>(data, offset + 44);
info.ownerId = readLittleEndian<uint32_t>(data, offset + 48);
           info.securityId = readLittleEndian<uint32_t>(data, offset + 52);
           info.quotaCharged = readLittleEndian<uint64_t>(data, offset + 56);
           info.updateSequenceNumber = readLittleEndian<uint64_t>(data, offset + 64);
       } else {
           info.maxVersions = 0;
           info.versionNumber = 0;
           info.classId = 0;
           info.ownerId = 0;
           info.securityId = 0;
           info.quotaCharged = 0;
           info.updateSequenceNumber = 0;
       }
       
       info.valid = true;
       return true;
   } catch (const std::exception& e) {
       info.valid = false;
       return false;
   }
}

std::string StandardInfoParser::getFileAttributesString(uint32_t attributes) {
   std::vector<std::string> attrStrings;
   
   if (attributes & FILE_ATTRIBUTE_READONLY) attrStrings.push_back("READONLY");
   if (attributes & FILE_ATTRIBUTE_HIDDEN) attrStrings.push_back("HIDDEN");
   if (attributes & FILE_ATTRIBUTE_SYSTEM) attrStrings.push_back("SYSTEM");
   if (attributes & FILE_ATTRIBUTE_DIRECTORY) attrStrings.push_back("DIRECTORY");
   if (attributes & FILE_ATTRIBUTE_ARCHIVE) attrStrings.push_back("ARCHIVE");
   if (attributes & FILE_ATTRIBUTE_DEVICE) attrStrings.push_back("DEVICE");
   if (attributes & FILE_ATTRIBUTE_NORMAL) attrStrings.push_back("NORMAL");
   if (attributes & FILE_ATTRIBUTE_TEMPORARY) attrStrings.push_back("TEMPORARY");
   if (attributes & FILE_ATTRIBUTE_SPARSE_FILE) attrStrings.push_back("SPARSE");
   if (attributes & FILE_ATTRIBUTE_REPARSE_POINT) attrStrings.push_back("REPARSE_POINT");
   if (attributes & FILE_ATTRIBUTE_COMPRESSED) attrStrings.push_back("COMPRESSED");
   if (attributes & FILE_ATTRIBUTE_OFFLINE) attrStrings.push_back("OFFLINE");
   if (attributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) attrStrings.push_back("NOT_INDEXED");
   if (attributes & FILE_ATTRIBUTE_ENCRYPTED) attrStrings.push_back("ENCRYPTED");
   
   if (attrStrings.empty()) {
       return "NONE";
   }
   
   std::string result = attrStrings[0];
   for (size_t i = 1; i < attrStrings.size(); ++i) {
       result += " | " + attrStrings[i];
   }
   return result;
}