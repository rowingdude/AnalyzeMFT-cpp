#include "mftRecord.h"
#include "../utils/hashCalc.h"
#include "../utils/stringUtils.h"
#include "../parsers/attributeParser.h"
#include "../parsers/standardinfoParser.h"
#include "../parsers/filenameParser.h"
#include "../parsers/objectidParser.h"
#include "../parsers/secdescParser.h"
#include "../parsers/volumeParser.h"
#include "../parsers/dataParser.h"
#include "../parsers/indexParser.h"
#include "../parsers/bitmapParser.h"
#include "../parsers/reparsepointParser.h"
#include "../parsers/xattrParser.h"
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

MftRecord::MftRecord(const std::vector<uint8_t>& rawRecord, bool computeHashes, int debugLevel)
    : rawRecord(rawRecord), debugLevel(debugLevel), computeHashesFlag(computeHashes),
      magic(0), updOff(0), updCnt(0), lsn(0), seq(0), link(0), attrOff(0), flags(0),
      size(0), allocSizef(0), baseRef(0), nextAttrid(0), recordnum(0), filesize(0), parentRef(0) {
    
    if (computeHashesFlag) {
        computeHashes();
    }
    parseRecord();
}

template<typename T>
T MftRecord::readLittleEndian(size_t offset) const {
    if (offset + sizeof(T) > rawRecord.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(rawRecord[offset + i]) << (i * 8);
    }
    return value;
}

void MftRecord::parseRecord() {
    if (rawRecord.size() < MFT_RECORD_SIZE) {
        return;
    }
    
    try {
        magic = readLittleEndian<uint32_t>(MFT_RECORD_MAGIC_NUMBER_OFFSET);
        updOff = readLittleEndian<uint16_t>(MFT_RECORD_UPDATE_SEQUENCE_OFFSET);
        updCnt = readLittleEndian<uint16_t>(MFT_RECORD_UPDATE_SEQUENCE_SIZE_OFFSET);
        lsn = readLittleEndian<uint64_t>(MFT_RECORD_LOGFILE_SEQUENCE_NUMBER_OFFSET);
        seq = readLittleEndian<uint16_t>(MFT_RECORD_SEQUENCE_NUMBER_OFFSET);
        link = readLittleEndian<uint16_t>(MFT_RECORD_HARD_LINK_COUNT_OFFSET);
        attrOff = readLittleEndian<uint16_t>(MFT_RECORD_FIRST_ATTRIBUTE_OFFSET);
        flags = readLittleEndian<uint16_t>(MFT_RECORD_FLAGS_OFFSET);
        size = readLittleEndian<uint32_t>(MFT_RECORD_USED_SIZE_OFFSET);
        allocSizef = readLittleEndian<uint32_t>(MFT_RECORD_ALLOCATED_SIZE_OFFSET);
        baseRef = readLittleEndian<uint64_t>(MFT_RECORD_FILE_REFERENCE_OFFSET);
        nextAttrid = readLittleEndian<uint16_t>(MFT_RECORD_NEXT_ATTRIBUTE_ID_OFFSET);
        recordnum = readLittleEndian<uint32_t>(MFT_RECORD_RECORD_NUMBER_OFFSET);
        
        parseAttributes();
    } catch (const std::exception& e) {
        if (debugLevel > 0) {
            // Error handling - could use logger here
        }
    }
}

void MftRecord::parseAttributes() {
    size_t offset = attrOff;
    
    while (offset < rawRecord.size() - 8) {
        try {
            uint32_t attrType = readLittleEndian<uint32_t>(offset);
            uint32_t attrLen = readLittleEndian<uint32_t>(offset + 4);
            
            if (attrType == 0xffffffff || attrLen == 0) {
                break;
            }
            
            attributeTypes.insert(attrType);
            
            switch (attrType) {
                case STANDARD_INFORMATION_ATTRIBUTE:
                    parseSiAttribute(offset);
                    break;
                case FILE_NAME_ATTRIBUTE:
                    parseFnAttribute(offset);
                    break;
                case ATTRIBUTE_LIST_ATTRIBUTE:
                    parseAttributeList(offset);
                    break;
                case OBJECT_ID_ATTRIBUTE:
                    parseObjectIdAttribute(offset);
                    break;
                case SECURITY_DESCRIPTOR_ATTRIBUTE:
                    parseSecurityDescriptor(offset);
                    break;
                case VOLUME_NAME_ATTRIBUTE:
                    parseVolumeName(offset);
                    break;
                case VOLUME_INFORMATION_ATTRIBUTE:
                    parseVolumeInformation(offset);
                    break;
                case DATA_ATTRIBUTE:
                    parseData(offset);
                    break;
                case INDEX_ROOT_ATTRIBUTE:
                    parseIndexRoot(offset);
                    break;
                case INDEX_ALLOCATION_ATTRIBUTE:
                    parseIndexAllocation(offset);
                    break;
                case BITMAP_ATTRIBUTE:
                    parseBitmap(offset);
                    break;
                case REPARSE_POINT_ATTRIBUTE:
                    parseReparsePoint(offset);
                    break;
                case EA_INFORMATION_ATTRIBUTE:
                    parseEaInformation(offset);
                    break;
                case EA_ATTRIBUTE:
                    parseEa(offset);
                    break;
                case LOGGED_UTILITY_STREAM_ATTRIBUTE:
                    parseLoggedUtilityStream(offset);
                    break;
            }
            
            offset += attrLen;
        } catch (const std::exception& e) {
            if (debugLevel >= 2) {
                // Log error
            }
            offset += 1;
        }
    }
}

void MftRecord::parseSiAttribute(size_t offset) {
    if (offset + 72 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 32 <= rawRecord.size()) {
        try {
            uint32_t crLow = readLittleEndian<uint32_t>(dataOffset);
            uint32_t crHigh = readLittleEndian<uint32_t>(dataOffset + 4);
            uint32_t mLow = readLittleEndian<uint32_t>(dataOffset + 8);
            uint32_t mHigh = readLittleEndian<uint32_t>(dataOffset + 12);
            uint32_t cLow = readLittleEndian<uint32_t>(dataOffset + 16);
            uint32_t cHigh = readLittleEndian<uint32_t>(dataOffset + 20);
            uint32_t aLow = readLittleEndian<uint32_t>(dataOffset + 24);
            uint32_t aHigh = readLittleEndian<uint32_t>(dataOffset + 28);
            
            siTimes.crtime = WindowsTime(crLow, crHigh);
            siTimes.mtime = WindowsTime(mLow, mHigh);
            siTimes.ctime = WindowsTime(cLow, cHigh);
            siTimes.atime = WindowsTime(aLow, aHigh);
        } catch (const std::exception& e) {
            // Handle error
        }
    }
}

void MftRecord::parseFnAttribute(size_t offset) {
    if (offset + 90 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 64 <= rawRecord.size()) {
        try {
            parentRef = readLittleEndian<uint64_t>(dataOffset) & 0x0000FFFFFFFFFFFF;
            
            uint32_t crLow = readLittleEndian<uint32_t>(dataOffset + 8);
            uint32_t crHigh = readLittleEndian<uint32_t>(dataOffset + 12);
            uint32_t mLow = readLittleEndian<uint32_t>(dataOffset + 16);
            uint32_t mHigh = readLittleEndian<uint32_t>(dataOffset + 20);
            uint32_t cLow = readLittleEndian<uint32_t>(dataOffset + 24);
            uint32_t cHigh = readLittleEndian<uint32_t>(dataOffset + 28);
            uint32_t aLow = readLittleEndian<uint32_t>(dataOffset + 32);
            uint32_t aHigh = readLittleEndian<uint32_t>(dataOffset + 36);
            
            fnTimes.crtime = WindowsTime(crLow, crHigh);
            fnTimes.mtime = WindowsTime(mLow, mHigh);
            fnTimes.ctime = WindowsTime(cLow, cHigh);
            fnTimes.atime = WindowsTime(aLow, aHigh);
            
            filesize = readLittleEndian<uint64_t>(dataOffset + 48);
            
            if (dataOffset + 64 < rawRecord.size()) {
                uint8_t nameLen = rawRecord[dataOffset + 64];
                if (dataOffset + 66 + nameLen * 2 <= rawRecord.size()) {
                    filename = readUtf16String(dataOffset + 66, nameLen);
                }
            }
        } catch (const std::exception& e) {
            // Handle error
        }
    }
}

void MftRecord::parseObjectIdAttribute(size_t offset) {
    if (offset + 88 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 64 <= rawRecord.size()) {
        try {
            objectId = bytesToGuid(&rawRecord[dataOffset]);
            birthVolumeId = bytesToGuid(&rawRecord[dataOffset + 16]);
            birthObjectId = bytesToGuid(&rawRecord[dataOffset + 32]);
            birthDomainId = bytesToGuid(&rawRecord[dataOffset + 48]);
        } catch (const std::exception& e) {
            if (debugLevel > 0) {
                // Log error
            }
        }
    }
}

void MftRecord::parseAttributeList(size_t offset) {
    uint16_t contentOffset = readLittleEndian<uint16_t>(offset + 20);
    uint32_t contentEnd = readLittleEndian<uint32_t>(offset + 4);
    
    size_t attrContentOffset = offset + contentOffset;
    size_t attrContentEnd = offset + contentEnd;
    
    while (attrContentOffset < attrContentEnd && attrContentOffset + 24 <= rawRecord.size()) {
        try {
            AttributeListEntry entry;
            entry.type = readLittleEndian<uint32_t>(attrContentOffset);
            uint16_t attrLen = readLittleEndian<uint16_t>(attrContentOffset + 4);
            uint8_t nameLen = rawRecord[attrContentOffset + 6];
            uint8_t nameOffset = rawRecord[attrContentOffset + 7];
            
            if (nameLen > 0 && attrContentOffset + nameOffset + nameLen * 2 <= rawRecord.size()) {
                entry.name = readUtf16String(attrContentOffset + nameOffset, nameLen);
            }
            
            entry.vcn = readLittleEndian<uint64_t>(attrContentOffset + 8);
            entry.reference = readLittleEndian<uint64_t>(attrContentOffset + 16);
            
            attributeList.push_back(entry);
            attrContentOffset += attrLen;
        } catch (const std::exception& e) {
            break;
        }
    }
}

void MftRecord::parseSecurityDescriptor(size_t offset) {
    if (offset + 44 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 20 <= rawRecord.size()) {
        try {
            securityDescriptor = std::make_unique<SecurityDescriptor>();
            securityDescriptor->revision = rawRecord[dataOffset];
            securityDescriptor->control = readLittleEndian<uint16_t>(dataOffset + 2);
            securityDescriptor->ownerOffset = readLittleEndian<uint32_t>(dataOffset + 4);
            securityDescriptor->groupOffset = readLittleEndian<uint32_t>(dataOffset + 8);
            securityDescriptor->saclOffset = readLittleEndian<uint32_t>(dataOffset + 12);
            securityDescriptor->daclOffset = readLittleEndian<uint32_t>(dataOffset + 16);
        } catch (const std::exception& e) {
            securityDescriptor.reset();
        }
    }
}

void MftRecord::parseVolumeName(size_t offset) {
    if (offset + 26 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    try {
        uint16_t nameLength = readLittleEndian<uint16_t>(dataOffset);
        if (dataOffset + 2 + nameLength * 2 <= rawRecord.size()) {
            volumeName = readUtf16String(dataOffset + 2, nameLength);
        }
    } catch (const std::exception& e) {
        // Handle error
    }
}

void MftRecord::parseVolumeInformation(size_t offset) {
    if (offset + 36 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 12 <= rawRecord.size()) {
        try {
            volumeInfo = std::make_unique<VolumeInfo>();
            volumeInfo->majorVersion = rawRecord[dataOffset + 8];
            volumeInfo->minorVersion = rawRecord[dataOffset + 9];
            volumeInfo->flags = readLittleEndian<uint16_t>(dataOffset + 10);
        } catch (const std::exception& e) {
            volumeInfo.reset();
        }
    }
}

void MftRecord::parseData(size_t offset) {
    if (offset + 24 > rawRecord.size()) return;
    
    try {
        dataAttribute = std::make_unique<DataAttribute>();
        uint8_t nonResidentFlag = rawRecord[offset + 8];
        uint8_t nameLength = rawRecord[offset + 9];
        uint16_t nameOffset = readLittleEndian<uint16_t>(offset + 10);
        
        if (nameLength > 0 && offset + nameOffset + nameLength * 2 <= rawRecord.size()) {
            dataAttribute->name = readUtf16String(offset + nameOffset, nameLength);
        }
        
        dataAttribute->nonResident = (nonResidentFlag != 0);
        
        if (!dataAttribute->nonResident) {
            dataAttribute->contentSize = readLittleEndian<uint32_t>(offset + 16);
        } else {
            dataAttribute->startVcn = readLittleEndian<uint64_t>(offset + 16);
            if (offset + 32 <= rawRecord.size()) {
                dataAttribute->lastVcn = readLittleEndian<uint64_t>(offset + 24);
            }
        }
    } catch (const std::exception& e) {
        dataAttribute.reset();
    }
}

void MftRecord::parseIndexRoot(size_t offset) {
    if (offset + 37 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 13 <= rawRecord.size()) {
        try {
            indexRoot = std::make_unique<IndexRoot>();
            indexRoot->attrType = readLittleEndian<uint32_t>(dataOffset);
            indexRoot->collationRule = readLittleEndian<uint32_t>(dataOffset + 4);
            indexRoot->indexAllocSize = readLittleEndian<uint32_t>(dataOffset + 8);
            indexRoot->clustersPerIndex = rawRecord[dataOffset + 12];
        } catch (const std::exception& e) {
            indexRoot.reset();
        }
    }
}

void MftRecord::parseIndexAllocation(size_t offset) {
    if (offset + 26 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 2 <= rawRecord.size()) {
        try {
            indexAllocation = std::make_unique<IndexAllocation>();
            indexAllocation->dataRunsOffset = readLittleEndian<uint16_t>(dataOffset);
        } catch (const std::exception& e) {
            indexAllocation.reset();
        }
    }
}

void MftRecord::parseBitmap(size_t offset) {
    if (offset + 28 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 4 <= rawRecord.size()) {
        try {
            bitmap = std::make_unique<BitmapAttribute>();
            bitmap->size = readLittleEndian<uint32_t>(dataOffset);
            if (dataOffset + 4 + bitmap->size <= rawRecord.size()) {
                bitmap->data.assign(rawRecord.begin() + dataOffset + 4, 
                                   rawRecord.begin() + dataOffset + 4 + bitmap->size);
            }
        } catch (const std::exception& e) {
            bitmap.reset();
        }
    }
}

void MftRecord::parseReparsePoint(size_t offset) {
    if (offset + 30 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 6 <= rawRecord.size()) {
        try {
            reparsePoint = std::make_unique<ReparsePoint>();
            reparsePoint->reparseTag = readLittleEndian<uint32_t>(dataOffset);
            reparsePoint->dataLength = readLittleEndian<uint16_t>(dataOffset + 4);
            if (dataOffset + 8 + reparsePoint->dataLength <= rawRecord.size()) {
                reparsePoint->data.assign(rawRecord.begin() + dataOffset + 8,
                                         rawRecord.begin() + dataOffset + 8 + reparsePoint->dataLength);
            }
        } catch (const std::exception& e) {
            reparsePoint.reset();
        }
    }
}

void MftRecord::parseEaInformation(size_t offset) {
    if (offset + 32 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 8 <= rawRecord.size()) {
        try {
            eaInformation = std::make_unique<EaInformation>();
            eaInformation->eaSize = readLittleEndian<uint32_t>(dataOffset);
            eaInformation->eaCount = readLittleEndian<uint32_t>(dataOffset + 4);
        } catch (const std::exception& e) {
            eaInformation.reset();
        }
    }
}

void MftRecord::parseEa(size_t offset) {
    if (offset + 32 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 8 <= rawRecord.size()) {
        try {
            ea = std::make_unique<ExtendedAttribute>();
            ea->nextEntryOffset = readLittleEndian<uint32_t>(dataOffset);
            ea->flags = rawRecord[dataOffset + 4];
            uint8_t nameLength = rawRecord[dataOffset + 5];
            uint16_t valueLength = readLittleEndian<uint16_t>(dataOffset + 6);
            
            if (dataOffset + 8 + nameLength <= rawRecord.size()) {
                ea->name.assign(rawRecord.begin() + dataOffset + 8,
                               rawRecord.begin() + dataOffset + 8 + nameLength);
            }
            
            if (dataOffset + 8 + nameLength + valueLength <= rawRecord.size()) {
                ea->value.assign(rawRecord.begin() + dataOffset + 8 + nameLength,
                                rawRecord.begin() + dataOffset + 8 + nameLength + valueLength);
            }
        } catch (const std::exception& e) {
            ea.reset();
        }
    }
}

void MftRecord::parseLoggedUtilityStream(size_t offset) {
    if (offset + 32 > rawRecord.size()) return;
    
    size_t dataOffset = offset + 24;
    if (dataOffset + 8 <= rawRecord.size()) {
        try {
            loggedUtilityStream = std::make_unique<LoggedUtilityStream>();
            loggedUtilityStream->size = readLittleEndian<uint64_t>(dataOffset);
            if (dataOffset + 8 + loggedUtilityStream->size <= rawRecord.size()) {
                loggedUtilityStream->data.assign(rawRecord.begin() + dataOffset + 8,
                                                rawRecord.begin() + dataOffset + 8 + loggedUtilityStream->size);
            }
        } catch (const std::exception& e) {
            loggedUtilityStream.reset();
        }
    }
}

std::string MftRecord::readUtf16String(size_t offset, size_t length) const {
    if (offset + length * 2 > rawRecord.size()) {
        return "";
    }
    
    std::wstring wstr;
    for (size_t i = 0; i < length; ++i) {
        uint16_t wchar = readLittleEndian<uint16_t>(offset + i * 2);
        wstr.push_back(static_cast<wchar_t>(wchar));
    }
    
    return StringUtils::wstringToString(wstr);
}

std::string MftRecord::bytesToGuid(const uint8_t* bytes) const {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    
    for (int i = 3; i >= 0; --i) oss << std::setw(2) << static_cast<int>(bytes[i]);
    oss << "-";
    for (int i = 5; i >= 4; --i) oss << std::setw(2) << static_cast<int>(bytes[i]);
    oss << "-";
    for (int i = 7; i >= 6; --i) oss << std::setw(2) << static_cast<int>(bytes[i]);
    oss << "-";
    for (int i = 8; i < 10; ++i) oss << std::setw(2) << static_cast<int>(bytes[i]);
    oss << "-";
    for (int i = 10; i < 16; ++i) oss << std::setw(2) << static_cast<int>(bytes[i]);
    
    return oss.str();
}

uint64_t MftRecord::getParentRecordNum() const {
    return parentRef & 0x0000FFFFFFFFFFFF;
}

std::string MftRecord::getFileType() const {
    if (flags & FILE_RECORD_IS_DIRECTORY) {
        return "Directory";
    } else if (flags & FILE_RECORD_IS_EXTENSION) {
        return "Extension";
    } else if (flags & FILE_RECORD_HAS_SPECIAL_INDEX) {
        return "Special Index";
    } else {
        return "File";
    }
}

void MftRecord::computeHashes() {
    if (!rawRecord.empty()) {
        HashCalculator calc;
        md5 = calc.calculateMd5(rawRecord);
        sha256 = calc.calculateSha256(rawRecord);
        sha512 = calc.calculateSha512(rawRecord);
        crc32 = calc.calculateCrc32(rawRecord);
    }
}

std::vector<std::string> MftRecord::toCsv() const {
    std::vector<std::string> row;
    
    row.push_back(std::to_string(recordnum));
    row.push_back((magic == MFT_RECORD_MAGIC) ? "Valid" : "Invalid");
    row.push_back((flags & FILE_RECORD_IN_USE) ? "In Use" : "Not in Use");
    row.push_back(getFileType());
    row.push_back(std::to_string(seq));
    row.push_back(std::to_string(getParentRecordNum()));
    row.push_back(std::to_string(baseRef >> 48));
    
    row.push_back(filename);
    row.push_back("");  // Filepath to be filled later
    
    row.push_back(siTimes.crtime.getDateTimeString());
    row.push_back(siTimes.mtime.getDateTimeString());
    row.push_back(siTimes.atime.getDateTimeString());
    row.push_back(siTimes.ctime.getDateTimeString());
    
    row.push_back(fnTimes.crtime.getDateTimeString());
    row.push_back(fnTimes.mtime.getDateTimeString());
    row.push_back(fnTimes.atime.getDateTimeString());
    row.push_back(fnTimes.ctime.getDateTimeString());
    
    row.push_back(objectId);
    row.push_back(birthVolumeId);
    row.push_back(birthObjectId);
    row.push_back(birthDomainId);
    
    row.push_back(attributeTypes.count(STANDARD_INFORMATION_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(ATTRIBUTE_LIST_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(FILE_NAME_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(VOLUME_NAME_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(VOLUME_INFORMATION_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(DATA_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(INDEX_ROOT_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(INDEX_ALLOCATION_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(BITMAP_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(REPARSE_POINT_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(EA_INFORMATION_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(EA_ATTRIBUTE) ? "True" : "False");
    row.push_back(attributeTypes.count(LOGGED_UTILITY_STREAM_ATTRIBUTE) ? "True" : "False");
    
    // Add detailed attribute information (simplified for now)
    row.push_back("");  // Attribute List Details
    row.push_back(securityDescriptor ? "Present" : "");
    row.push_back(volumeName);
    row.push_back(volumeInfo ? "Present" : "");
    row.push_back(dataAttribute ? "Present" : "");
    row.push_back(indexRoot ? "Present" : "");
    row.push_back(indexAllocation ? "Present" : "");
    row.push_back(bitmap ? "Present" : "");
    row.push_back(reparsePoint ? "Present" : "");
    row.push_back(eaInformation ? "Present" : "");
    row.push_back(ea ? "Present" : "");
    row.push_back(loggedUtilityStream ? "Present" : "");
    
    if (computeHashesFlag) {
        row.push_back(md5);
        row.push_back(sha256);
        row.push_back(sha512);
        row.push_back(crc32);
    } else {
        row.push_back("");
        row.push_back("");
        row.push_back("");
        row.push_back("");
    }
    
    return row;
}