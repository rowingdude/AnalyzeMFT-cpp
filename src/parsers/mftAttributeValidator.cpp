**src/parsers/mftAttributeValidator.cpp**
```cpp
#include "../../include/parsers/mftAttributeValidator.h"
#include "../core/mftRecord.h"
#include "../utils/stringUtils.h"
#include <iostream>
#include <sstream>
#include <iomanip>

MftAttributeValidator::MftAttributeValidator(int debugLevel, uint32_t recordNumber)
    : debugLevel(debugLevel), recordNumber(recordNumber) {
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateStandardInformation(
    const std::vector<uint8_t>& data, size_t offset,
    WindowsTime& crtime, WindowsTime& mtime, WindowsTime& atime, WindowsTime& ctime) {
    
    ValidationHelpers::AttributeHeader header;
    auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
    if (!headerResult.isValid) {
        logValidationError("Standard Information header validation failed: " + headerResult.errorMessage);
        return headerResult;
    }
    
    if (header.type != 0x10) {
        return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Standard Information");
    }
    
    // Standard Information minimum size is 48 bytes (basic timestamps + file attributes)
    if (header.nonResident != 0) {
        logValidationError("Standard Information attribute cannot be non-resident");
        return ValidationHelpers::ValidationResult(false, "Standard Information cannot be non-resident");
    }
    
    if (header.valueLength < 48) {
        logValidationError("Standard Information attribute too small: " + std::to_string(header.valueLength) + " bytes");
        return ValidationHelpers::ValidationResult(false, "Standard Information attribute too small");
    }
    
    size_t dataOffset = offset + header.valueOffset;
    auto boundsResult = ValidationHelpers::validateBounds(data, dataOffset, 48);
    if (!boundsResult.isValid) {
        logValidationError("Standard Information data bounds check failed: " + boundsResult.errorMessage);
        return boundsResult;
    }
    
    try {
        bool success = true;
        
        // Read timestamps
        uint32_t crLow = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset, success);
        uint32_t crHigh = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 4, success);
        uint32_t mLow = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 8, success);
        uint32_t mHigh = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 12, success);
        uint32_t aLow = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 16, success);
        uint32_t aHigh = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 20, success);
        uint32_t cLow = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 24, success);
        uint32_t cHigh = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 28, success);
        
        if (!success) {
            logValidationError("Failed to read Standard Information timestamps");
            return ValidationHelpers::ValidationResult(false, "Failed to read timestamps");
        }
        
        // Validate timestamps
        auto crResult = ValidationHelpers::validateTimestamp(crLow, crHigh);
        auto mResult = ValidationHelpers::validateTimestamp(mLow, mHigh);
        auto aResult = ValidationHelpers::validateTimestamp(aLow, aHigh);
        auto cResult = ValidationHelpers::validateTimestamp(cLow, cHigh);
        
        if (!crResult.isValid) logValidationWarning("Invalid creation timestamp: " + crResult.errorMessage);
        if (!mResult.isValid) logValidationWarning("Invalid modification timestamp: " + mResult.errorMessage);
        if (!aResult.isValid) logValidationWarning("Invalid access timestamp: " + aResult.errorMessage);
        if (!cResult.isValid) logValidationWarning("Invalid change timestamp: " + cResult.errorMessage);
        
        // Create WindowsTime objects
        crtime = WindowsTime(crLow, crHigh);
        mtime = WindowsTime(mLow, mHigh);
        atime = WindowsTime(aLow, aHigh);
        ctime = WindowsTime(cLow, cHigh);
        
        // Read and validate file attributes
        uint32_t fileAttributes = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 32, success);
        if (!success) {
            logValidationError("Failed to read file attributes");
            return ValidationHelpers::ValidationResult(false, "Failed to read file attributes");
        }
        
        // Validate file attributes for consistency
        const uint32_t VALID_ATTRIBUTES_MASK = 0x00007FF7; // Known valid attribute bits
        if ((fileAttributes & ~VALID_ATTRIBUTES_MASK) != 0) {
            logValidationWarning("Unknown file attribute bits set: 0x" + 
                std::to_string(fileAttributes & ~VALID_ATTRIBUTES_MASK));
        }
        
        logValidationInfo("Standard Information validation successful");
        return ValidationHelpers::ValidationResult(true, "", header.length);
        
    } catch (const std::exception& e) {
        logValidationError("Exception during Standard Information validation: " + std::string(e.what()));
        return ValidationHelpers::ValidationResult(false, "Exception during validation");
    }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateFileName(
    const std::vector<uint8_t>& data, size_t offset,
    std::string& filename, WindowsTime& crtime, WindowsTime& mtime, WindowsTime& atime, WindowsTime& ctime,
    uint64_t& filesize, uint64_t& parentRef) {
    
    ValidationHelpers::AttributeHeader header;
    auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
    if (!headerResult.isValid) {
        logValidationError("File Name header validation failed: " + headerResult.errorMessage);
        return headerResult;
    }
    
    if (header.type != 0x30) {
        return ValidationHelpers::ValidationResult(false, "Invalid attribute type for File Name");
    }
    
    if (header.nonResident != 0) {
        logValidationError("File Name attribute cannot be non-resident");
        return ValidationHelpers::ValidationResult(false, "File Name cannot be non-resident");
    }
    
    // File Name minimum size is 66 bytes (fixed header + at least 1 character name)
    if (header.valueLength < 66) {
        logValidationError("File Name attribute too small: " + std::to_string(header.valueLength) + " bytes");
        return ValidationHelpers::ValidationResult(false, "File Name attribute too small");
    }
    
    size_t dataOffset = offset + header.valueOffset;
    auto boundsResult = ValidationHelpers::validateBounds(data, dataOffset, 66);
    if (!boundsResult.isValid) {
        logValidationError("File Name data bounds check failed: " + boundsResult.errorMessage);
        return boundsResult;
    }
    
    try {
        bool success = true;
        
        // Read parent directory reference
        parentRef = ValidationHelpers::readLittleEndianSafe<uint64_t>(data, dataOffset, success);
        if (!success) {
            logValidationError("Failed to read parent directory reference");
            return ValidationHelpers::ValidationResult(false, "Failed to read parent reference");
        }
        
        // Validate parent reference
        auto parentResult = ValidationHelpers::validateFileReference(parentRef);
        if (!parentResult.isValid) {
            logValidationWarning("Invalid parent reference: " + parentResult.errorMessage);
        }
        
        // Read timestamps
        uint32_t crLow = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 8, success);
        uint32_t crHigh = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 12, success);
        uint32_t mLow = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 16, success);
        uint32_t mHigh = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 20, success);
        uint32_t aLow = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 24, success);
        uint32_t aHigh = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 28, success);
        uint32_t cLow = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 32, success);
        uint32_t cHigh = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 36, success);
        
        if (!success) {
            logValidationError("Failed to read File Name timestamps");
            return ValidationHelpers::ValidationResult(false, "Failed to read timestamps");
        }
        
        // Create WindowsTime objects (with less strict validation than SI)
        crtime = WindowsTime(crLow, crHigh);
        mtime = WindowsTime(mLow, mHigh);
        atime = WindowsTime(aLow, aHigh);
        ctime = WindowsTime(cLow, cHigh);
        
        // Read file sizes
        uint64_t allocatedSize = ValidationHelpers::readLittleEndianSafe<uint64_t>(data, dataOffset + 40, success);
        filesize = ValidationHelpers::readLittleEndianSafe<uint64_t>(data, dataOffset + 48, success);
        
        if (!success) {
            logValidationError("Failed to read file sizes");
            return ValidationHelpers::ValidationResult(false, "Failed to read file sizes");
        }
        
        // Validate size consistency
        if (filesize > allocatedSize) {
            logValidationWarning("File size (" + std::to_string(filesize) + 
                ") exceeds allocated size (" + std::to_string(allocatedSize) + ")");
        }
        
        // Read filename metadata
        uint8_t filenameLength = data[dataOffset + 64];
        uint8_t filenameNamespace = data[dataOffset + 65];
        
        if (filenameLength == 0) {
            logValidationError("Zero filename length");
            return ValidationHelpers::ValidationResult(false, "Zero filename length");
        }
        
        if (filenameLength > ValidationHelpers::MAX_FILENAME_LENGTH) {
            logValidationError("Filename length too large: " + std::to_string(filenameLength));
            return ValidationHelpers::ValidationResult(false, "Filename length too large");
        }
        
        // Validate filename namespace
        if (filenameNamespace > 3) {
            logValidationWarning("Invalid filename namespace: " + std::to_string(filenameNamespace));
        }
        
        // Validate filename bounds
        size_t filenameOffset = dataOffset + 66;
        auto filenameResult = ValidationHelpers::validateUtf16String(data, filenameOffset, filenameLength);
        if (!filenameResult.isValid) {
            logValidationError("Filename validation failed: " + filenameResult.errorMessage);
            return ValidationHelpers::ValidationResult(false, "Invalid filename");
        }
        
        // Read filename
        filename = ValidationHelpers::readUtf16StringSafe(data, filenameOffset, filenameLength, success);
        if (!success) {
            logValidationError("Failed to read filename");
            return ValidationHelpers::ValidationResult(false, "Failed to read filename");
        }
        
        // Validate filename content
        if (filename.empty() && filenameLength > 0) {
            logValidationWarning("Empty filename despite non-zero length");
        }
        
        // Check for invalid filename characters
        const std::string invalidChars = "<>:\"/\\|?*";
        for (char c : invalidChars) {
            if (filename.find(c) != std::string::npos) {
                logValidationWarning("Filename contains invalid character: " + std::string(1, c));
                break;
            }
        }
        
        logValidationInfo("File Name validation successful for: " + filename);
        return ValidationHelpers::ValidationResult(true, "", header.length);
        
    } catch (const std::exception& e) {
        logValidationError("Exception during File Name validation: " + std::string(e.what()));
        return ValidationHelpers::ValidationResult(false, "Exception during validation");
    }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateObjectId(
    const std::vector<uint8_t>& data, size_t offset,
    std::string& objectId, std::string& birthVolumeId, std::string& birthObjectId, std::string& birthDomainId) {
    
    ValidationHelpers::AttributeHeader header;
    auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
    if (!headerResult.isValid) {
        logValidationError("Object ID header validation failed: " + headerResult.errorMessage);
        return headerResult;
    }
    
    if (header.type != 0x40) {
        return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Object ID");
    }
    
    if (header.nonResident != 0) {
        logValidationError("Object ID attribute cannot be non-resident");
        return ValidationHelpers::ValidationResult(false, "Object ID cannot be non-resident");
    }
    
    // Object ID is exactly 64 bytes (4 GUIDs of 16 bytes each)
    if (header.valueLength != 64) {
        logValidationError("Object ID attribute invalid size: " + std::to_string(header.valueLength) + " bytes, expected 64");
        return ValidationHelpers::ValidationResult(false, "Object ID attribute wrong size");
    }
    
    size_t dataOffset = offset + header.valueOffset;
    auto boundsResult = ValidationHelpers::validateBounds(data, dataOffset, 64);
    if (!boundsResult.isValid) {
        logValidationError("Object ID data bounds check failed: " + boundsResult.errorMessage);
        return boundsResult;
    }
    
    try {
        bool success = true;
        
        // Validate and read each GUID
        auto guid1Result = ValidationHelpers::validateGuid(data, dataOffset);
        auto guid2Result = ValidationHelpers::validateGuid(data, dataOffset + 16);
        auto guid3Result = ValidationHelpers::validateGuid(data, dataOffset + 32);
        auto guid4Result = ValidationHelpers::validateGuid(data, dataOffset + 48);
        
        if (!guid1Result.isValid || !guid2Result.isValid || !guid3Result.isValid || !guid4Result.isValid) {
            logValidationError("One or more GUIDs in Object ID are invalid");
            return ValidationHelpers::ValidationResult(false, "Invalid GUID in Object ID");
        }
        
        objectId = ValidationHelpers::bytesToGuidSafe(data, dataOffset, success);
        if (!success) {
            logValidationError("Failed to read Object ID GUID");
            return ValidationHelpers::ValidationResult(false, "Failed to read Object ID");
        }
        
        birthVolumeId = ValidationHelpers::bytesToGuidSafe(data, dataOffset + 16, success);
        if (!success) {
            logValidationError("Failed to read Birth Volume ID GUID");
            return ValidationHelpers::ValidationResult(false, "Failed to read Birth Volume ID");
        }
        
        birthObjectId = ValidationHelpers::bytesToGuidSafe(data, dataOffset + 32, success);
        if (!success) {
            logValidationError("Failed to read Birth Object ID GUID");
            return ValidationHelpers::ValidationResult(false, "Failed to read Birth Object ID");
        }
        
        birthDomainId = ValidationHelpers::bytesToGuidSafe(data, dataOffset + 48, success);
        if (!success) {
            logValidationError("Failed to read Birth Domain ID GUID");
            return ValidationHelpers::ValidationResult(false, "Failed to read Birth Domain ID");
        }
        
        logValidationInfo("Object ID validation successful");
        return ValidationHelpers::ValidationResult(true, "", header.length);
        
    } catch (const std::exception& e) {
        logValidationError("Exception during Object ID validation: " + std::string(e.what()));
        return ValidationHelpers::ValidationResult(false, "Exception during validation");
    }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateAttributeList(
    const std::vector<uint8_t>& data, size_t offset,
    std::vector<AttributeListEntry>& attributeList) {
    
    ValidationHelpers::AttributeHeader header;
    auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
    if (!headerResult.isValid) {
        logValidationError("Attribute List header validation failed: " + headerResult.errorMessage);
        return headerResult;
    }
    
    if (header.type != 0x20) {
        return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Attribute List");
    }
    
    size_t dataOffset, dataSize;
    if (header.nonResident == 0) {
        // Resident attribute list
        dataOffset = offset + header.valueOffset;
        dataSize = header.valueLength;
    } else {
        // Non-resident attribute list - more complex parsing needed
        logValidationWarning("Non-resident Attribute List not fully supported yet");
        return ValidationHelpers::ValidationResult(true, "Non-resident attribute list skipped", header.length);
    }
    
    auto boundsResult = ValidationHelpers::validateBounds(data, dataOffset, dataSize);
    if (!boundsResult.isValid) {
        logValidationError("Attribute List data bounds check failed: " + boundsResult.errorMessage);
        return boundsResult;
    }
    
    try {
        attributeList.clear();
        size_t currentOffset = dataOffset;
        size_t endOffset = dataOffset + dataSize;
        uint32_t entryCount = 0;
        const uint32_t MAX_ATTRIBUTE_LIST_ENTRIES = 1000; // Safety limit
        
        while (currentOffset < endOffset && entryCount < MAX_ATTRIBUTE_LIST_ENTRIES) {
            // Minimum attribute list entry size is 24 bytes
            if (currentOffset + 24 > endOffset) {
                break;
            }
            
            bool success = true;
            AttributeListEntry entry;
            
            entry.type = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, currentOffset, success);
            uint16_t recordLength = ValidationHelpers::readLittleEndianSafe<uint16_t>(data, currentOffset + 4, success);
            uint8_t nameLength = data[currentOffset + 6];
            uint8_t nameOffset = data[currentOffset + 7];
            entry.vcn = ValidationHelpers::readLittleEndianSafe<uint64_t>(data, currentOffset + 8, success);
            entry.reference = ValidationHelpers::readLittleEndianSafe<uint64_t>(data, currentOffset + 16, success);
            
            if (!success) {
                logValidationError("Failed to read attribute list entry header");
                break;
            }
            
            // Validate record length
            if (recordLength < 24 || currentOffset + recordLength > endOffset) {
                logValidationError("Invalid attribute list entry length: " + std::to_string(recordLength));
                break;
            }
            
            // Validate attribute type
            if (!ValidationHelpers::isValidAttributeType(entry.type)) {
                logValidationWarning("Unknown attribute type in list: 0x" + std::to_string(entry.type));
            }
            
            // Validate file reference
            auto refResult = ValidationHelpers::validateFileReference(entry.reference);
            if (!refResult.isValid) {
                logValidationWarning("Invalid file reference in attribute list: " + refResult.errorMessage);
            }
            
            // Read attribute name if present
            if (nameLength > 0) {
                if (nameOffset >= recordLength || currentOffset + nameOffset + nameLength * 2 > endOffset) {
                    logValidationError("Invalid name offset/length in attribute list entry");
                    break;
                }
                
                entry.name = ValidationHelpers::readUtf16StringSafe(data, currentOffset + nameOffset, nameLength, success);
                if (!success) {
                    logValidationWarning("Failed to read attribute name in list entry");
                    entry.name = "";
                }
            }
            
            attributeList.push_back(entry);
            currentOffset += recordLength;
            entryCount++;
        }
        
        if (entryCount >= MAX_ATTRIBUTE_LIST_ENTRIES) {
            logValidationWarning("Attribute list entry limit reached");
        }
        
        logValidationInfo("Attribute List validation successful, " + std::to_string(attributeList.size()) + " entries");
        return ValidationHelpers::ValidationResult(true, "", header.length);
        
    } catch (const std::exception& e) {
        logValidationError("Exception during Attribute List validation: " + std::string(e.what()));
        return ValidationHelpers::ValidationResult(false, "Exception during validation");
    }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateSecurityDescriptor(
    const std::vector<uint8_t>& data, size_t offset,
    std::unique_ptr<SecurityDescriptor>& securityDescriptor) {
    
    ValidationHelpers::AttributeHeader header;
    auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
    if (!headerResult.isValid) {
        logValidationError("Security Descriptor header validation failed: " + headerResult.errorMessage);
        return headerResult;
    }
    
    if (header.type != 0x50) {
        return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Security Descriptor");
    }
    
    if (header.nonResident != 0) {
        logValidationError("Security Descriptor attribute cannot be non-resident");
        return ValidationHelpers::ValidationResult(false, "Security Descriptor cannot be non-resident");
    }
    
    // Security Descriptor minimum size is 20 bytes (header)
    if (header.valueLength < 20) {
        logValidationError("Security Descriptor attribute too small: " + std::to_string(header.valueLength) + " bytes");
        return ValidationHelpers::ValidationResult(false, "Security Descriptor too small");
    }
    
    size_t dataOffset = offset + header.valueOffset;
    auto boundsResult = ValidationHelpers::validateBounds(data, dataOffset, 20);
    if (!boundsResult.isValid) {
        logValidationError("Security Descriptor data bounds check failed: " + boundsResult.errorMessage);
        return boundsResult;
    }
    
    try {
        bool success = true;
        securityDescriptor = std::make_unique<SecurityDescriptor>();
        
        securityDescriptor->revision = data[dataOffset];
        securityDescriptor->control = ValidationHelpers::readLittleEndianSafe<uint16_t>(data, dataOffset + 2, success);
        securityDescriptor->ownerOffset = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 4, success);
        securityDescriptor->groupOffset = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 8, success);
        securityDescriptor->saclOffset = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 12, success);
        securityDescriptor->daclOffset = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 16, success);
        
        if (!success) {
            logValidationError("Failed to read Security Descriptor header");
            securityDescriptor.reset();
            return ValidationHelpers::ValidationResult(false, "Failed to read Security Descriptor header");
        }
        
        // Validate revision
        if (securityDescriptor->revision != 1) {
            logValidationWarning("Unexpected Security Descriptor revision: " + std::to_string(securityDescriptor->revision));
        }
        
        // Validate offsets are within bounds
        if (securityDescriptor->ownerOffset >= header.valueLength ||
            securityDescriptor->groupOffset >= header.valueLength ||
            (securityDescriptor->saclOffset != 0 && securityDescriptor->saclOffset >= header.valueLength) ||
            (securityDescriptor->daclOffset != 0 && securityDescriptor->daclOffset >= header.valueLength)) {
            logValidationError("Security Descriptor offset out of bounds");
            securityDescriptor.reset();
            return ValidationHelpers::ValidationResult(false, "Invalid Security Descriptor offsets");
        }
        
        // TODO: Add SID and ACL validation when those parsers are implemented
        
        logValidationInfo("Security Descriptor validation successful");
        return ValidationHelpers::ValidationResult(true, "", header.length);
        
    } catch (const std::exception& e) {
        logValidationError("Exception during Security Descriptor validation: " + std::string(e.what()));
        securityDescriptor.reset();
        return ValidationHelpers::ValidationResult(false, "Exception during validation");
    }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateVolumeName(
    const std::vector<uint8_t>& data, size_t offset, std::string& volumeName) {
    
    ValidationHelpers::AttributeHeader header;
    auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
    if (!headerResult.isValid) {
        logValidationError("Volume Name header validation failed: " + headerResult.errorMessage);
        return headerResult;
    }
    
    if (header.type != 0x60) {
        return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Volume Name");
    }
    
    if (header.nonResident != 0) {
        logValidationError("Volume Name attribute cannot be non-resident");
        return ValidationHelpers::ValidationResult(false, "Volume Name cannot be non-resident");
    }
    
    if (header.valueLength == 0) {
        volumeName = "";
        logValidationInfo("Empty Volume Name");
        return ValidationHelpers::ValidationResult(true, "", header.length);
    }
    
    if (header.valueLength > ValidationHelpers::MAX_VOLUME_NAME_LENGTH * 2) {
        logValidationError("Volume Name too long: " + std::to_string(header.valueLength / 2) + " characters");
        return ValidationHelpers::ValidationResult(false, "Volume Name too long");
    }
    
    size_t dataOffset = offset + header.valueOffset;
    size_t nameLength = header.valueLength / 2; // UTF-16 characters
    
    auto stringResult = ValidationHelpers::validateUtf16String(data, dataOffset, nameLength);
    if (!stringResult.isValid) {
        logValidationError("Volume Name string validation failed: " + stringResult.errorMessage);
        return stringResult;
    }
    
    try {
        bool success = true;
        volumeName = ValidationHelpers::readUtf16StringSafe(data, dataOffset, nameLength, success);
        if (!success) {
            logValidationError("Failed to read Volume Name");
            return ValidationHelpers::ValidationResult(false, "Failed to read Volume Name");
        }
        
        // Validate volume name content
        if (volumeName.empty() && header.valueLength > 0) {
            logValidationWarning("Empty Volume Name despite non-zero length");
        }
        
        logValidationInfo("Volume Name validation successful: " + volumeName);
        return ValidationHelpers::ValidationResult(true, "", header.length);
        
    } catch (const std::exception& e) {
        logValidationError("Exception during Volume Name validation: " + std::string(e.what()));
        return ValidationHelpers::ValidationResult(false, "Exception during validation");
    }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateVolumeInformation(
    const std::vector<uint8_t>& data, size_t offset, std::unique_ptr<VolumeInfo>& volumeInfo) {
    
    ValidationHelpers::AttributeHeader header;
    auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
    if (!headerResult.isValid) {
        logValidationError("Volume Information header validation failed: " + headerResult.errorMessage);
        return headerResult;
    }
    
    if (header.type != 0x70) {
        return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Volume Information");
    }
    
    if (header.nonResident != 0) {
        logValidationError("Volume Information attribute cannot be non-resident");
        return ValidationHelpers::ValidationResult(false, "Volume Information cannot be non-resident");
    }
    
    // Volume Information is exactly 12 bytes
    if (header.valueLength != 12) {
        logValidationError("Volume Information invalid size: " + std::to_string(header.valueLength) + " bytes, expected 12");
        return ValidationHelpers::ValidationResult(false, "Volume Information wrong size");
    }
    
    size_t dataOffset = offset + header.valueOffset;
    auto boundsResult = ValidationHelpers::validateBounds(data, dataOffset, 12);
    if (!boundsResult.isValid) {
        logValidationError("Volume Information data bounds check failed: " + boundsResult.errorMessage);
        return boundsResult;
    }
    
    try {
        bool success = true;
        volumeInfo = std::make_unique<VolumeInfo>();
        
        // Skip reserved field (8 bytes)
        volumeInfo->majorVersion = data[dataOffset + 8];
        volumeInfo->minorVersion = data[dataOffset + 9];
        volumeInfo->flags = ValidationHelpers::readLittleEndianSafe<uint16_t>(data, dataOffset + 10, success);
        
        if (!success) {
            logValidationError("Failed to read Volume Information");
            volumeInfo.reset();
            return ValidationHelpers::ValidationResult(false, "Failed to read Volume Information");
        }
        
        // Validate version numbers (NTFS versions are typically 3.x)
        if (volumeInfo->majorVersion < 1 || volumeInfo->majorVersion > 10) {
            logValidationWarning("Unusual NTFS major version: " + std::to_string(volumeInfo->majorVersion));
        }
        
        logValidationInfo("Volume Information validation successful (v" + 
            std::to_string(volumeInfo->majorVersion) + "." + std::to_string(volumeInfo->minorVersion) + ")");
        return ValidationHelpers::ValidationResult(true, "", header.length);
        
    } catch (const std::exception& e) {
        logValidationError("Exception during Volume Information validation: " + std::string(e.what()));
        volumeInfo.reset();
        return ValidationHelpers::ValidationResult(false, "Exception during validation");
    }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateData(
    const std::vector<uint8_t>& data, size_t offset, std::unique_ptr<DataAttribute>& dataAttribute) {
    
    ValidationHelpers::AttributeHeader header;
    auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
    if (!headerResult.isValid) {
        return headerResult;
    }
    
if (header.type != 0x80) {
       return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Data");
   }
   
   try {
       dataAttribute = std::make_unique<DataAttribute>();
       dataAttribute->nonResident = (header.nonResident != 0);
       
       // Read attribute name if present
       if (header.nameLength > 0) {
           bool success = true;
           dataAttribute->name = ValidationHelpers::readUtf16StringSafe(data, offset + header.nameOffset, header.nameLength, success);
           if (!success) {
               logValidationWarning("Failed to read Data attribute name");
               dataAttribute->name = "";
           }
       }
       
       if (!dataAttribute->nonResident) {
           // Resident data attribute
           dataAttribute->contentSize = header.valueLength;
           dataAttribute->startVcn = 0;
           dataAttribute->lastVcn = 0;
       } else {
           // Non-resident data attribute
           ValidationHelpers::NonResidentHeader nrHeader;
           auto nrResult = ValidationHelpers::validateNonResidentHeader(data, offset, nrHeader);
           if (!nrResult.isValid) {
               logValidationWarning("Non-resident Data header validation failed: " + nrResult.errorMessage);
           } else {
               dataAttribute->startVcn = nrHeader.startingVcn;
               dataAttribute->lastVcn = nrHeader.lastVcn;
               dataAttribute->contentSize = static_cast<uint32_t>(nrHeader.actualSize);
           }
       }
       
       logValidationInfo("Data attribute validation successful (" + 
           (dataAttribute->nonResident ? "non-resident" : "resident") + ")");
       return ValidationHelpers::ValidationResult(true, "", header.length);
       
   } catch (const std::exception& e) {
       logValidationError("Exception during Data validation: " + std::string(e.what()));
       dataAttribute.reset();
       return ValidationHelpers::ValidationResult(false, "Exception during validation");
   }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateIndexRoot(
   const std::vector<uint8_t>& data, size_t offset, std::unique_ptr<IndexRoot>& indexRoot) {
   
   ValidationHelpers::AttributeHeader header;
   auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
   if (!headerResult.isValid) {
       return headerResult;
   }
   
   if (header.type != 0x90) {
       return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Index Root");
   }
   
   if (header.nonResident != 0) {
       logValidationError("Index Root attribute cannot be non-resident");
       return ValidationHelpers::ValidationResult(false, "Index Root cannot be non-resident");
   }
   
   if (header.valueLength < 16) {
       logValidationError("Index Root attribute too small: " + std::to_string(header.valueLength) + " bytes");
       return ValidationHelpers::ValidationResult(false, "Index Root too small");
   }
   
   try {
       bool success = true;
       size_t dataOffset = offset + header.valueOffset;
       
       indexRoot = std::make_unique<IndexRoot>();
       indexRoot->attrType = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset, success);
       indexRoot->collationRule = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 4, success);
       indexRoot->indexAllocSize = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 8, success);
       indexRoot->clustersPerIndex = data[dataOffset + 12];
       
       if (!success) {
           logValidationError("Failed to read Index Root header");
           indexRoot.reset();
           return ValidationHelpers::ValidationResult(false, "Failed to read Index Root");
       }
       
       // Basic validation
       if (!ValidationHelpers::isValidAttributeType(indexRoot->attrType)) {
           logValidationWarning("Unknown indexed attribute type: 0x" + std::to_string(indexRoot->attrType));
       }
       
       if (indexRoot->indexAllocSize == 0 || indexRoot->indexAllocSize > 65536) {
           logValidationWarning("Unusual index allocation size: " + std::to_string(indexRoot->indexAllocSize));
       }
       
       logValidationInfo("Index Root validation successful");
       return ValidationHelpers::ValidationResult(true, "", header.length);
       
   } catch (const std::exception& e) {
       logValidationError("Exception during Index Root validation: " + std::string(e.what()));
       indexRoot.reset();
       return ValidationHelpers::ValidationResult(false, "Exception during validation");
   }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateIndexAllocation(
   const std::vector<uint8_t>& data, size_t offset, std::unique_ptr<IndexAllocation>& indexAllocation) {
   
   ValidationHelpers::AttributeHeader header;
   auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
   if (!headerResult.isValid) {
       return headerResult;
   }
   
   if (header.type != 0xA0) {
       return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Index Allocation");
   }
   
   if (header.nonResident == 0) {
       logValidationError("Index Allocation attribute must be non-resident");
       return ValidationHelpers::ValidationResult(false, "Index Allocation must be non-resident");
   }
   
   try {
       ValidationHelpers::NonResidentHeader nrHeader;
       auto nrResult = ValidationHelpers::validateNonResidentHeader(data, offset, nrHeader);
       if (!nrResult.isValid) {
           logValidationError("Index Allocation non-resident header validation failed: " + nrResult.errorMessage);
           return nrResult;
       }
       
       indexAllocation = std::make_unique<IndexAllocation>();
       indexAllocation->dataRunsOffset = nrHeader.dataRunsOffset;
       
       logValidationInfo("Index Allocation validation successful");
       return ValidationHelpers::ValidationResult(true, "", header.length);
       
   } catch (const std::exception& e) {
       logValidationError("Exception during Index Allocation validation: " + std::string(e.what()));
       indexAllocation.reset();
       return ValidationHelpers::ValidationResult(false, "Exception during validation");
   }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateBitmap(
   const std::vector<uint8_t>& data, size_t offset, std::unique_ptr<BitmapAttribute>& bitmap) {
   
   ValidationHelpers::AttributeHeader header;
   auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
   if (!headerResult.isValid) {
       return headerResult;
   }
   
   if (header.type != 0xB0) {
       return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Bitmap");
   }
   
   try {
       bitmap = std::make_unique<BitmapAttribute>();
       
       if (header.nonResident == 0) {
           // Resident bitmap
           bitmap->size = header.valueLength;
           if (bitmap->size > 0) {
               size_t dataOffset = offset + header.valueOffset;
               auto boundsResult = ValidationHelpers::validateBounds(data, dataOffset, bitmap->size);
               if (boundsResult.isValid) {
                   bitmap->data.assign(data.begin() + dataOffset, data.begin() + dataOffset + bitmap->size);
               }
           }
       } else {
           // Non-resident bitmap - would need data run parsing
           ValidationHelpers::NonResidentHeader nrHeader;
           auto nrResult = ValidationHelpers::validateNonResidentHeader(data, offset, nrHeader);
           if (nrResult.isValid) {
               bitmap->size = static_cast<uint32_t>(nrHeader.actualSize);
           }
       }
       
       logValidationInfo("Bitmap validation successful");
       return ValidationHelpers::ValidationResult(true, "", header.length);
       
   } catch (const std::exception& e) {
       logValidationError("Exception during Bitmap validation: " + std::string(e.what()));
       bitmap.reset();
       return ValidationHelpers::ValidationResult(false, "Exception during validation");
   }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateReparsePoint(
   const std::vector<uint8_t>& data, size_t offset, std::unique_ptr<ReparsePoint>& reparsePoint) {
   
   ValidationHelpers::AttributeHeader header;
   auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
   if (!headerResult.isValid) {
       return headerResult;
   }
   
   if (header.type != 0xC0) {
       return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Reparse Point");
   }
   
   if (header.nonResident != 0) {
       logValidationError("Reparse Point attribute cannot be non-resident");
       return ValidationHelpers::ValidationResult(false, "Reparse Point cannot be non-resident");
   }
   
   if (header.valueLength < 8) {
       logValidationError("Reparse Point attribute too small: " + std::to_string(header.valueLength) + " bytes");
       return ValidationHelpers::ValidationResult(false, "Reparse Point too small");
   }
   
   try {
       bool success = true;
       size_t dataOffset = offset + header.valueOffset;
       
       reparsePoint = std::make_unique<ReparsePoint>();
       reparsePoint->reparseTag = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset, success);
       reparsePoint->dataLength = ValidationHelpers::readLittleEndianSafe<uint16_t>(data, dataOffset + 4, success);
       
       if (!success) {
           logValidationError("Failed to read Reparse Point header");
           reparsePoint.reset();
           return ValidationHelpers::ValidationResult(false, "Failed to read Reparse Point");
       }
       
       // Validate data length
       if (reparsePoint->dataLength > header.valueLength - 8) {
           logValidationError("Reparse Point data length exceeds attribute size");
           reparsePoint.reset();
           return ValidationHelpers::ValidationResult(false, "Invalid Reparse Point data length");
       }
       
       // Read reparse data if present
       if (reparsePoint->dataLength > 0) {
           auto boundsResult = ValidationHelpers::validateBounds(data, dataOffset + 8, reparsePoint->dataLength);
           if (boundsResult.isValid) {
               reparsePoint->data.assign(data.begin() + dataOffset + 8, 
                                        data.begin() + dataOffset + 8 + reparsePoint->dataLength);
           }
       }
       
       logValidationInfo("Reparse Point validation successful (tag: 0x" + 
           std::to_string(reparsePoint->reparseTag) + ")");
       return ValidationHelpers::ValidationResult(true, "", header.length);
       
   } catch (const std::exception& e) {
       logValidationError("Exception during Reparse Point validation: " + std::string(e.what()));
       reparsePoint.reset();
       return ValidationHelpers::ValidationResult(false, "Exception during validation");
   }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateEaInformation(
   const std::vector<uint8_t>& data, size_t offset, std::unique_ptr<EaInformation>& eaInformation) {
   
   ValidationHelpers::AttributeHeader header;
   auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
   if (!headerResult.isValid) {
       return headerResult;
   }
   
   if (header.type != 0xD0) {
       return ValidationHelpers::ValidationResult(false, "Invalid attribute type for EA Information");
   }
   
   if (header.nonResident != 0) {
       logValidationError("EA Information attribute cannot be non-resident");
       return ValidationHelpers::ValidationResult(false, "EA Information cannot be non-resident");
   }
   
   if (header.valueLength != 8) {
       logValidationError("EA Information invalid size: " + std::to_string(header.valueLength) + " bytes, expected 8");
       return ValidationHelpers::ValidationResult(false, "EA Information wrong size");
   }
   
   try {
       bool success = true;
       size_t dataOffset = offset + header.valueOffset;
       
       eaInformation = std::make_unique<EaInformation>();
       eaInformation->eaSize = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset, success);
       eaInformation->eaCount = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset + 4, success);
       
       if (!success) {
           logValidationError("Failed to read EA Information");
           eaInformation.reset();
           return ValidationHelpers::ValidationResult(false, "Failed to read EA Information");
       }
       
       logValidationInfo("EA Information validation successful");
       return ValidationHelpers::ValidationResult(true, "", header.length);
       
   } catch (const std::exception& e) {
       logValidationError("Exception during EA Information validation: " + std::string(e.what()));
       eaInformation.reset();
       return ValidationHelpers::ValidationResult(false, "Exception during validation");
   }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateEa(
   const std::vector<uint8_t>& data, size_t offset, std::unique_ptr<ExtendedAttribute>& ea) {
   
   ValidationHelpers::AttributeHeader header;
   auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
   if (!headerResult.isValid) {
       return headerResult;
   }
   
   if (header.type != 0xE0) {
       return ValidationHelpers::ValidationResult(false, "Invalid attribute type for EA");
   }
   
   if (header.nonResident != 0) {
       logValidationError("EA attribute cannot be non-resident");
       return ValidationHelpers::ValidationResult(false, "EA cannot be non-resident");
   }
   
   if (header.valueLength < 8) {
       logValidationError("EA attribute too small: " + std::to_string(header.valueLength) + " bytes");
       return ValidationHelpers::ValidationResult(false, "EA too small");
   }
   
   try {
       bool success = true;
       size_t dataOffset = offset + header.valueOffset;
       
       ea = std::make_unique<ExtendedAttribute>();
       ea->nextEntryOffset = ValidationHelpers::readLittleEndianSafe<uint32_t>(data, dataOffset, success);
       ea->flags = data[dataOffset + 4];
       uint8_t nameLength = data[dataOffset + 5];
       uint16_t valueLength = ValidationHelpers::readLittleEndianSafe<uint16_t>(data, dataOffset + 6, success);
       
       if (!success) {
           logValidationError("Failed to read EA header");
           ea.reset();
           return ValidationHelpers::ValidationResult(false, "Failed to read EA");
       }
       
       // Read name if present
       if (nameLength > 0 && dataOffset + 8 + nameLength <= data.size()) {
           ea->name.assign(data.begin() + dataOffset + 8, data.begin() + dataOffset + 8 + nameLength);
       }
       
       // Read value if present
       if (valueLength > 0) {
           size_t valueOffset = dataOffset + 8 + nameLength;
           if (valueOffset + valueLength <= data.size()) {
               ea->value.assign(data.begin() + valueOffset, data.begin() + valueOffset + valueLength);
           }
       }
       
       logValidationInfo("EA validation successful");
       return ValidationHelpers::ValidationResult(true, "", header.length);
       
   } catch (const std::exception& e) {
       logValidationError("Exception during EA validation: " + std::string(e.what()));
       ea.reset();
       return ValidationHelpers::ValidationResult(false, "Exception during validation");
   }
}

ValidationHelpers::ValidationResult MftAttributeValidator::validateLoggedUtilityStream(
   const std::vector<uint8_t>& data, size_t offset, std::unique_ptr<LoggedUtilityStream>& loggedUtilityStream) {
   
   ValidationHelpers::AttributeHeader header;
   auto headerResult = ValidationHelpers::validateAttributeHeader(data, offset, header);
   if (!headerResult.isValid) {
       return headerResult;
   }
   
   if (header.type != 0x100) {
       return ValidationHelpers::ValidationResult(false, "Invalid attribute type for Logged Utility Stream");
   }
   
   try {
       loggedUtilityStream = std::make_unique<LoggedUtilityStream>();
       
       if (header.nonResident == 0) {
           // Resident stream
           loggedUtilityStream->size = header.valueLength;
           if (loggedUtilityStream->size > 0) {
               size_t dataOffset = offset + header.valueOffset;
               auto boundsResult = ValidationHelpers::validateBounds(data, dataOffset, loggedUtilityStream->size);
               if (boundsResult.isValid) {
                   loggedUtilityStream->data.assign(data.begin() + dataOffset, 
                                                   data.begin() + dataOffset + loggedUtilityStream->size);
               }
           }
       } else {
           // Non-resident stream
           ValidationHelpers::NonResidentHeader nrHeader;
           auto nrResult = ValidationHelpers::validateNonResidentHeader(data, offset, nrHeader);
           if (nrResult.isValid) {
               loggedUtilityStream->size = nrHeader.actualSize;
           }
       }
       
       logValidationInfo("Logged Utility Stream validation successful");
       return ValidationHelpers::ValidationResult(true, "", header.length);
       
   } catch (const std::exception& e) {
       logValidationError("Exception during Logged Utility Stream validation: " + std::string(e.what()));
       loggedUtilityStream.reset();
       return ValidationHelpers::ValidationResult(false, "Exception during validation");
   }
}

// Logging methods
void MftAttributeValidator::logValidationError(const std::string& message, int level) const {
   if (level <= debugLevel) {
       std::cerr << "[ERROR] Record " << recordNumber << ": " << message << std::endl;
   }
}

void MftAttributeValidator::logValidationWarning(const std::string& message) const {
   if (debugLevel >= 1) {
       std::cout << "[WARN] Record " << recordNumber << ": " << message << std::endl;
   }
}

void MftAttributeValidator::logValidationInfo(const std::string& message) const {
   if (debugLevel >= 2) {
       std::cout << "[INFO] Record " << recordNumber << ": " << message << std::endl;
   }
}