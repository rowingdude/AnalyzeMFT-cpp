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

// Update constructor
MftRecord::MftRecord(const std::vector<uint8_t>& rawRecord, bool computeHashes, int debugLevel)
    : rawRecord(rawRecord), debugLevel(debugLevel), computeHashesFlag(computeHashes),
      magic(0), updOff(0), updCnt(0), lsn(0), seq(0), link(0), attrOff(0), flags(0),
      size(0), allocSizef(0), baseRef(0), nextAttrid(0), recordnum(0), filesize(0), parentRef(0) {
    
    // Initialize validator
    validator = std::make_unique<MftAttributeValidator>(debugLevel, 0); // recordnum set later
    
    if (computeHashesFlag) {
        computeHashes();
    }
    parseRecord();
}

// Validation wrapper implementations
bool MftRecord::parseSiAttributeWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateStandardInformation(rawRecord, offset, 
        siTimes.crtime, siTimes.mtime, siTimes.atime, siTimes.ctime);
    
    if (!result.isValid && debugLevel > 0) {
        log("Standard Information validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseFnAttributeWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateFileName(rawRecord, offset, 
        filename, fnTimes.crtime, fnTimes.mtime, fnTimes.atime, fnTimes.ctime, filesize, parentRef);
    
    if (!result.isValid && debugLevel > 0) {
        log("File Name validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseObjectIdAttributeWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateObjectId(rawRecord, offset, 
        objectId, birthVolumeId, birthObjectId, birthDomainId);
    
    if (!result.isValid && debugLevel > 0) {
        log("Object ID validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseAttributeListWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateAttributeList(rawRecord, offset, attributeList);
    
    if (!result.isValid && debugLevel > 0) {
        log("Attribute List validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseSecurityDescriptorWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateSecurityDescriptor(rawRecord, offset, securityDescriptor);
    
    if (!result.isValid && debugLevel > 0) {
        log("Security Descriptor validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseVolumeNameWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateVolumeName(rawRecord, offset, volumeName);
    
    if (!result.isValid && debugLevel > 0) {
        log("Volume Name validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseVolumeInformationWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateVolumeInformation(rawRecord, offset, volumeInfo);
    
    if (!result.isValid && debugLevel > 0) {
        log("Volume Information validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseDataWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateData(rawRecord, offset, dataAttribute);
    
    if (!result.isValid && debugLevel > 0) {
        log("Data validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseIndexRootWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateIndexRoot(rawRecord, offset, indexRoot);
    
    if (!result.isValid && debugLevel > 0) {
        log("Index Root validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseIndexAllocationWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateIndexAllocation(rawRecord, offset, indexAllocation);
    
    if (!result.isValid && debugLevel > 0) {
        log("Index Allocation validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseBitmapWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateBitmap(rawRecord, offset, bitmap);
    
    if (!result.isValid && debugLevel > 0) {
        log("Bitmap validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseReparsePointWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateReparsePoint(rawRecord, offset, reparsePoint);
    
    if (!result.isValid && debugLevel > 0) {
        log("Reparse Point validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseEaInformationWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateEaInformation(rawRecord, offset, eaInformation);
    
    if (!result.isValid && debugLevel > 0) {
        log("EA Information validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseEaWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateEa(rawRecord, offset, ea);
    
    if (!result.isValid && debugLevel > 0) {
        log("EA validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}

bool MftRecord::parseLoggedUtilityStreamWithValidation(size_t offset) {
    validator->setRecordNumber(recordnum);
    auto result = validator->validateLoggedUtilityStream(rawRecord, offset, loggedUtilityStream);
    
    if (!result.isValid && debugLevel > 0) {
        log("Logged Utility Stream validation failed: " + result.errorMessage, 1);
    }
    
    return result.isValid;
}



void MftRecord::parseRecord() {
    if (rawRecord.size() < MFT_RECORD_SIZE) {
        if (debugLevel > 0) {
            log("Invalid MFT record size: " + std::to_string(rawRecord.size()) + 
                " bytes, expected " + std::to_string(MFT_RECORD_SIZE), 1);
        }
        return;
    }
    
    try {
        magic = readLittleEndian<uint32_t>(MFT_RECORD_MAGIC_NUMBER_OFFSET);
        if (magic != MFT_RECORD_MAGIC) {
            if (debugLevel > 1) {
                log("Invalid MFT record magic: 0x" + std::to_string(magic) + 
                    ", expected 0x" + std::to_string(MFT_RECORD_MAGIC) + 
                    " for record " + std::to_string(recordnum), 2);
            }
        }
        
        updOff = readLittleEndian<uint16_t>(MFT_RECORD_UPDATE_SEQUENCE_OFFSET);
        updCnt = readLittleEndian<uint16_t>(MFT_RECORD_UPDATE_SEQUENCE_SIZE_OFFSET);
        
        if (updOff < 42 || updOff >= MFT_RECORD_SIZE || 
            updCnt == 0 || updOff + (updCnt * 2) > MFT_RECORD_SIZE) {
            if (debugLevel > 1) {
                log("Invalid update sequence array: offset=" + std::to_string(updOff) + 
                    ", count=" + std::to_string(updCnt) + 
                    " for record " + std::to_string(recordnum), 2);
            }
            return;
        }
        
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
        
        if (size > MFT_RECORD_SIZE || allocSizef != MFT_RECORD_SIZE) {
            if (debugLevel > 1) {
                log("Invalid record size: used=" + std::to_string(size) + 
                    ", allocated=" + std::to_string(allocSizef) + 
                    " for record " + std::to_string(recordnum), 2);
            }
        }
        
        if (attrOff < 56 || attrOff >= size) {
            if (debugLevel > 1) {
                log("Invalid attribute offset: " + std::to_string(attrOff) + 
                    " for record " + std::to_string(recordnum), 2);
            }
            attrOff = 56; // Set to minimum valid offset
        }
        
        if (!applyFixupArray()) {
            if (debugLevel > 0) {
                log("Fixup array validation failed for record " + std::to_string(recordnum), 1);
            }
        }
        
        parseAttributes();
        
    } catch (const std::exception& e) {
        if (debugLevel > 0) {
            log("Exception parsing MFT record header for record " + 
                std::to_string(recordnum) + ": " + e.what(), 1);
        }
    }
}

template<typename T>
T MftRecord::readLittleEndian(size_t offset) const {
    if (offset + sizeof(T) > rawRecord.size()) {
        if (debugLevel > 2) {
            log("Read beyond record bounds: offset=" + std::to_string(offset) + 
                ", size=" + std::to_string(sizeof(T)) + 
                ", record_size=" + std::to_string(rawRecord.size()) + 
                " for record " + std::to_string(recordnum), 3);
        }
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T) && (offset + i) < rawRecord.size(); ++i) {
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
    uint32_t attributeCount = 0;
    const uint32_t MAX_ATTRIBUTES = 100; // Safety limit
    
    while (offset < rawRecord.size() - 8 && attributeCount < MAX_ATTRIBUTES) {
        try {
            if (debugLevel > 2) {
                log("Parsing attribute at offset " + std::to_string(offset) + 
                    " for record " + std::to_string(recordnum), 3);
            }
            
            uint32_t attrType = readLittleEndian<uint32_t>(offset);
            uint32_t attrLen = readLittleEndian<uint32_t>(offset + 4);
            
            if (debugLevel > 2) {
                log("Attribute type: 0x" + std::to_string(attrType) + 
                    ", length: " + std::to_string(attrLen) + 
                    " for record " + std::to_string(recordnum), 3);
            }

            // Check for end of attributes
            if (attrType == 0xffffffff || attrLen == 0) {
                if (debugLevel > 2) {
                    log("End of attributes reached for record " + std::to_string(recordnum), 3);
                }
                break;
            }
            
            // Validate attribute length
            if (attrLen < 16 || attrLen > (rawRecord.size() - offset)) {
                if (debugLevel > 1) {
                    log("Invalid attribute length: " + std::to_string(attrLen) + 
                        " at offset " + std::to_string(offset) + 
                        " for record " + std::to_string(recordnum), 2);
                }
                break;
            }
            
            // Ensure attribute length is properly aligned
            if (attrLen % 8 != 0) {
                if (debugLevel > 2) {
                    log("Attribute length not 8-byte aligned: " + std::to_string(attrLen) + 
                        " for record " + std::to_string(recordnum), 3);
                }
            }
            
            attributeTypes.insert(attrType);
            attributeCount++;

            // Parse specific attribute types with enhanced error handling
            bool parseSuccess = false;
            switch (attrType) {
                case STANDARD_INFORMATION_ATTRIBUTE:
                    parseSuccess = parseSiAttributeWithValidation(offset);
                    break;
                case FILE_NAME_ATTRIBUTE:
                    parseSuccess = parseFnAttributeWithValidation(offset);
                    break;
                case ATTRIBUTE_LIST_ATTRIBUTE:
                    parseSuccess = parseAttributeListWithValidation(offset);
                    break;
                case OBJECT_ID_ATTRIBUTE:
                    parseSuccess = parseObjectIdAttributeWithValidation(offset);
                    break;
                case SECURITY_DESCRIPTOR_ATTRIBUTE:
                    parseSuccess = parseSecurityDescriptorWithValidation(offset);
                    break;
                case VOLUME_NAME_ATTRIBUTE:
                    parseSuccess = parseVolumeNameWithValidation(offset);
                    break;
                case VOLUME_INFORMATION_ATTRIBUTE:
                    parseSuccess = parseVolumeInformationWithValidation(offset);
                    break;
                case DATA_ATTRIBUTE:
                    parseSuccess = parseDataWithValidation(offset);
                    break;
                case INDEX_ROOT_ATTRIBUTE:
                    parseSuccess = parseIndexRootWithValidation(offset);
                    break;
                case INDEX_ALLOCATION_ATTRIBUTE:
                    parseSuccess = parseIndexAllocationWithValidation(offset);
                    break;
                case BITMAP_ATTRIBUTE:
                    parseSuccess = parseBitmapWithValidation(offset);
                    break;
                case REPARSE_POINT_ATTRIBUTE:
                    parseSuccess = parseReparsePointWithValidation(offset);
                    break;
                case EA_INFORMATION_ATTRIBUTE:
                    parseSuccess = parseEaInformationWithValidation(offset);
                    break;
                case EA_ATTRIBUTE:
                    parseSuccess = parseEaWithValidation(offset);
                    break;
                case LOGGED_UTILITY_STREAM_ATTRIBUTE:
                    parseSuccess = parseLoggedUtilityStreamWithValidation(offset);
                    break;
                default:
                    if (debugLevel > 1) {
                        log("Unknown attribute type: 0x" + std::to_string(attrType) + 
                            " at offset " + std::to_string(offset) + 
                            " for record " + std::to_string(recordnum), 2);
                    }
                    parseSuccess = true; // Continue parsing
                    break;
            }
            
            if (!parseSuccess && debugLevel > 1) {
                log("Failed to parse attribute type 0x" + std::to_string(attrType) + 
                    " for record " + std::to_string(recordnum), 2);
            }

            offset += attrLen;

        } catch (const std::exception& e) {
            if (debugLevel >= 1) {
                log("Exception processing attribute at offset " + std::to_string(offset) + 
                    " for record " + std::to_string(recordnum) + ": " + e.what(), 1);
            }
            offset += 16; // Skip minimal attribute header size
        }
    }
    
    if (attributeCount >= MAX_ATTRIBUTES) {
        if (debugLevel > 0) {
            log("Maximum attribute limit reached for record " + std::to_string(recordnum), 1);
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

bool MftRecord::applyFixupArray() {
    if (updCnt == 0 || updOff == 0) {
        return true;
    }
    
    try {
        uint16_t updateSeqNum = readLittleEndian<uint16_t>(updOff);
        
        if (debugLevel > 2) {
            log("Applying fixup array: USN=0x" + std::to_string(updateSeqNum) + 
                ", count=" + std::to_string(updCnt) + 
                " for record " + std::to_string(recordnum), 3);
        }
        
        for (uint16_t i = 1; i < updCnt; ++i) {
            size_t sectorOffset = (i * 512) - 2; // Last 2 bytes of each sector
            
            if (sectorOffset + 2 > rawRecord.size()) {
                if (debugLevel > 1) {
                    log("Fixup sector offset out of bounds: " + std::to_string(sectorOffset) + 
                        " for record " + std::to_string(recordnum), 2);
                }
                return false;
            }
            
            uint16_t sectorSeqNum = readLittleEndian<uint16_t>(sectorOffset);
            if (sectorSeqNum != updateSeqNum) {
                if (debugLevel > 1) {
                    log("Fixup validation failed: expected 0x" + std::to_string(updateSeqNum) + 
                        ", found 0x" + std::to_string(sectorSeqNum) + 
                        " at sector " + std::to_string(i) + 
                        " for record " + std::to_string(recordnum), 2);
                }
                return false;
            }
            
            uint16_t fixupValue = readLittleEndian<uint16_t>(updOff + (i * 2));
            rawRecord[sectorOffset] = static_cast<uint8_t>(fixupValue & 0xFF);
            rawRecord[sectorOffset + 1] = static_cast<uint8_t>((fixupValue >> 8) & 0xFF);
        }
        
        return true;
    } catch (const std::exception& e) {
        if (debugLevel > 0) {
            log("Exception in fixup array processing for record " + 
                std::to_string(recordnum) + ": " + e.what(), 1);
        }
        return false;
    }
}

void MftRecord::log(const std::string& message, int level) const {
    if (level <= debugLevel) {
        std::cout << message << std::endl;
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