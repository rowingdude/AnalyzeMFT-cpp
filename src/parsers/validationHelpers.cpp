#include "../../include/parsers/validationHelpers.h"
#include "../utils/stringUtils.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

ValidationHelpers::ValidationResult ValidationHelpers::validateBounds(const std::vector<uint8_t>& data, size_t offset, size_t requiredSize) {
    if (offset >= data.size()) {
        return ValidationResult(false, "Offset beyond data bounds", 0);
    }
    
    if (offset + requiredSize > data.size()) {
        return ValidationResult(false, 
            "Required size " + std::to_string(requiredSize) + 
            " exceeds available data at offset " + std::to_string(offset), 0);
    }
    
    return ValidationResult(true, "", requiredSize);
}

ValidationHelpers::ValidationResult ValidationHelpers::validateAttributeHeader(const std::vector<uint8_t>& data, size_t offset, AttributeHeader& header) {
    auto boundsResult = validateBounds(data, offset, MIN_ATTRIBUTE_SIZE);
    if (!boundsResult.isValid) {
        return boundsResult;
    }
    
    bool success = true;
    header.type = readLittleEndianSafe<uint32_t>(data, offset, success);
    if (!success) return ValidationResult(false, "Failed to read attribute type");
    
    header.length = readLittleEndianSafe<uint32_t>(data, offset + 4, success);
    if (!success) return ValidationResult(false, "Failed to read attribute length");
    
    // Validate attribute type
    if (!isValidAttributeType(header.type)) {
        return ValidationResult(false, "Invalid attribute type: 0x" + std::to_string(header.type));
    }
    
    // Validate attribute length
    if (header.length < MIN_ATTRIBUTE_SIZE || header.length > MAX_ATTRIBUTE_SIZE) {
        return ValidationResult(false, "Invalid attribute length: " + std::to_string(header.length));
    }
    
    // Ensure we have enough data for the full attribute
    auto fullBoundsResult = validateBounds(data, offset, header.length);
    if (!fullBoundsResult.isValid) {
        return ValidationResult(false, "Attribute extends beyond available data");
    }
    
    header.nonResident = data[offset + 8];
    header.nameLength = data[offset + 9];
    header.nameOffset = readLittleEndianSafe<uint16_t>(data, offset + 10, success);
    header.flags = readLittleEndianSafe<uint16_t>(data, offset + 12, success);
    header.attributeId = readLittleEndianSafe<uint16_t>(data, offset + 14, success);
    
    if (header.nonResident == 0) {
        // Resident attribute
        header.valueLength = readLittleEndianSafe<uint32_t>(data, offset + 16, success);
        header.valueOffset = readLittleEndianSafe<uint16_t>(data, offset + 20, success);
        header.indexedFlag = data[offset + 22];
        header.padding = data[offset + 23];
        
        // Validate resident attribute fields
        if (header.valueOffset >= header.length) {
            return ValidationResult(false, "Invalid value offset in resident attribute");
        }
        
        if (header.valueOffset + header.valueLength > header.length) {
            return ValidationResult(false, "Value extends beyond attribute length");
        }
    }
    
    // Validate attribute name if present
    if (header.nameLength > 0) {
        if (header.nameOffset >= header.length) {
            return ValidationResult(false, "Invalid name offset");
        }
        
        auto nameResult = validateUtf16String(data, offset + header.nameOffset, header.nameLength);
        if (!nameResult.isValid) {
            return ValidationResult(false, "Invalid attribute name: " + nameResult.errorMessage);
        }
    }
    
    header.valid = true;
    return ValidationResult(true, "", header.length);
}

ValidationHelpers::ValidationResult ValidationHelpers::validateNonResidentHeader(const std::vector<uint8_t>& data, size_t offset, NonResidentHeader& header) {
    auto boundsResult = validateBounds(data, offset + 16, 48); // Non-resident header starts at offset 16
    if (!boundsResult.isValid) {
        return boundsResult;
    }
    
    bool success = true;
    header.startingVcn = readLittleEndianSafe<uint64_t>(data, offset + 16, success);
    header.lastVcn = readLittleEndianSafe<uint64_t>(data, offset + 24, success);
    header.dataRunsOffset = readLittleEndianSafe<uint16_t>(data, offset + 32, success);
    header.compressionUnit = readLittleEndianSafe<uint16_t>(data, offset + 34, success);
    header.padding = readLittleEndianSafe<uint32_t>(data, offset + 36, success);
    header.allocatedSize = readLittleEndianSafe<uint64_t>(data, offset + 40, success);
    header.actualSize = readLittleEndianSafe<uint64_t>(data, offset + 48, success);
    header.initializedSize = readLittleEndianSafe<uint64_t>(data, offset + 56, success);
    
    if (!success) {
        return ValidationResult(false, "Failed to read non-resident header fields");
    }
    
    // Validate VCN range
    if (header.startingVcn > header.lastVcn) {
        return ValidationResult(false, "Invalid VCN range: start > last");
    }
    
    // Validate size consistency
    if (header.actualSize > header.allocatedSize) {
        return ValidationResult(false, "Actual size exceeds allocated size");
    }
    
    if (header.initializedSize > header.actualSize) {
        return ValidationResult(false, "Initialized size exceeds actual size");
    }
    
    header.valid = true;
    return ValidationResult(true, "", 48);
}

ValidationHelpers::ValidationResult ValidationHelpers::validateUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t lengthInChars) {
    if (lengthInChars == 0) {
        return ValidationResult(true, "", 0);
    }
    
    size_t requiredBytes = lengthInChars * 2;
    auto boundsResult = validateBounds(data, offset, requiredBytes);
    if (!boundsResult.isValid) {
        return boundsResult;
    }
    
    // Check for valid UTF-16 sequences
    for (size_t i = 0; i < lengthInChars; ++i) {
        size_t charOffset = offset + (i * 2);
        uint16_t wchar = readLittleEndianSafe<uint16_t>(data, charOffset, bool());
        
        // Basic UTF-16 validation - check for invalid surrogates
        if ((wchar >= 0xD800 && wchar <= 0xDBFF)) { // High surrogate
            if (i + 1 >= lengthInChars) {
                return ValidationResult(false, "Incomplete surrogate pair at end of string");
            }
            
            uint16_t lowSurrogate = readLittleEndianSafe<uint16_t>(data, charOffset + 2, bool());
            if (!(lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF)) {
                return ValidationResult(false, "Invalid low surrogate in UTF-16 string");
            }
            i++; // Skip the low surrogate
        } else if (wchar >= 0xDC00 && wchar <= 0xDFFF) {
            return ValidationResult(false, "Unexpected low surrogate in UTF-16 string");
        }
    }
    
    return ValidationResult(true, "", requiredBytes);
}

ValidationHelpers::ValidationResult ValidationHelpers::validateGuid(const std::vector<uint8_t>& data, size_t offset) {
    auto boundsResult = validateBounds(data, offset, 16);
    if (!boundsResult.isValid) {
        return boundsResult;
    }
    
    // GUIDs can be any 16-byte value, so just check that we can read it
    bool success = true;
    bytesToGuidSafe(data, offset, success);
    
    if (!success) {
        return ValidationResult(false, "Failed to parse GUID");
    }
    
    return ValidationResult(true, "", 16);
}

ValidationHelpers::ValidationResult ValidationHelpers::validateTimestamp(uint32_t low, uint32_t high) {
    if (low == 0 && high == 0) {
        return ValidationResult(true, "", 0); // Zero timestamp is valid (means not set)
    }
    
    // Check for reasonable timestamp range (1601-2200 approximately)
    uint64_t timestamp = (static_cast<uint64_t>(high) << 32) | low;
    
    // Windows epoch starts at 1601-01-01, Unix epoch at 1970-01-01
    // Minimum reasonable value: 1980-01-01 (approximation)
    const uint64_t MIN_REASONABLE_TIMESTAMP = 119600064000000000ULL;
    // Maximum reasonable value: 2200-01-01 (approximation)  
    const uint64_t MAX_REASONABLE_TIMESTAMP = 138506112000000000ULL;
    
    if (timestamp < MIN_REASONABLE_TIMESTAMP || timestamp > MAX_REASONABLE_TIMESTAMP) {
        return ValidationResult(false, "Timestamp outside reasonable range");
    }
    
    return ValidationResult(true, "", 8);
}

ValidationHelpers::ValidationResult ValidationHelpers::validateFileReference(uint64_t reference) {
    uint64_t recordNumber = reference & 0x0000FFFFFFFFFFFF;
    uint16_t sequenceNumber = static_cast<uint16_t>((reference >> 48) & 0xFFFF);
    
    if (!isValidMftRecordNumber(recordNumber)) {
        return ValidationResult(false, "Invalid MFT record number: " + std::to_string(recordNumber));
    }
    
    if (sequenceNumber == 0) {
        return ValidationResult(false, "Invalid sequence number: 0");
    }
    
    return ValidationResult(true, "", 8);
}

template<typename T>
T ValidationHelpers::readLittleEndianSafe(const std::vector<uint8_t>& data, size_t offset, bool& success) {
    success = true;
    
    if (offset + sizeof(T) > data.size()) {
        success = false;
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

std::string ValidationHelpers::readUtf16StringSafe(const std::vector<uint8_t>& data, size_t offset, size_t lengthInChars, bool& success) {
    success = true;
    
    auto validation = validateUtf16String(data, offset, lengthInChars);
    if (!validation.isValid) {
        success = false;
        return "";
    }
    
    std::wstring wstr;
    for (size_t i = 0; i < lengthInChars; ++i) {
        uint16_t wchar = readLittleEndianSafe<uint16_t>(data, offset + i * 2, success);
        if (!success) return "";
        
        if (wchar == 0) break; // Null terminator
        wstr.push_back(static_cast<wchar_t>(wchar));
    }
    
    return StringUtils::wstringToString(wstr);
}

std::string ValidationHelpers::bytesToGuidSafe(const std::vector<uint8_t>& data, size_t offset, bool& success) {
    success = true;
    
    if (offset + 16 > data.size()) {
        success = false;
        return "";
    }
    
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    
    // GUID format: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    // Little-endian for first three parts, big-endian for last two
    
    // Data1 (4 bytes, little-endian)
    for (int i = 3; i >= 0; --i) {
        oss << std::setw(2) << static_cast<int>(data[offset + i]);
    }
    oss << "-";
    
    // Data2 (2 bytes, little-endian)
    for (int i = 5; i >= 4; --i) {
        oss << std::setw(2) << static_cast<int>(data[offset + i]);
    }
    oss << "-";
    
    // Data3 (2 bytes, little-endian)
    for (int i = 7; i >= 6; --i) {
        oss << std::setw(2) << static_cast<int>(data[offset + i]);
    }
    oss << "-";
    
    // Data4 (8 bytes, big-endian)
    for (int i = 8; i < 10; ++i) {
        oss << std::setw(2) << static_cast<int>(data[offset + i]);
    }
    oss << "-";
    
    for (int i = 10; i < 16; ++i) {
        oss << std::setw(2) << static_cast<int>(data[offset + i]);
    }
    
    return oss.str();
}

bool ValidationHelpers::isValidMftRecordNumber(uint64_t recordNumber) {
    return recordNumber <= MAX_MFT_RECORD_NUMBER && recordNumber != 0;
}

bool ValidationHelpers::isValidAttributeType(uint32_t attributeType) {
    // Standard NTFS attribute types
    switch (attributeType) {
        case 0x10: // $STANDARD_INFORMATION
        case 0x20: // $ATTRIBUTE_LIST  
        case 0x30: // $FILE_NAME
        case 0x40: // $OBJECT_ID
        case 0x50: // $SECURITY_DESCRIPTOR
        case 0x60: // $VOLUME_NAME
        case 0x70: // $VOLUME_INFORMATION
        case 0x80: // $DATA
        case 0x90: // $INDEX_ROOT
        case 0xA0: // $INDEX_ALLOCATION
        case 0xB0: // $BITMAP
        case 0xC0: // $REPARSE_POINT
        case 0xD0: // $EA_INFORMATION
        case 0xE0: // $EA
        case 0x100: // $LOGGED_UTILITY_STREAM
            return true;
        default:
            // Allow unknown attributes but they might be vendor-specific
            return attributeType != 0xFFFFFFFF; // End marker
    }
}

std::string ValidationHelpers::getValidationErrorMessage(const std::string& context, const std::string& error, size_t offset, uint32_t recordNumber) {
    return context + " validation failed for record " + std::to_string(recordNumber) + 
           " at offset " + std::to_string(offset) + ": " + error;
}

// Explicit template instantiations
template uint8_t ValidationHelpers::readLittleEndianSafe<uint8_t>(const std::vector<uint8_t>&, size_t, bool&);
template uint16_t ValidationHelpers::readLittleEndianSafe<uint16_t>(const std::vector<uint8_t>&, size_t, bool&);
template uint32_t ValidationHelpers::readLittleEndianSafe<uint32_t>(const std::vector<uint8_t>&, size_t, bool&);
template uint64_t ValidationHelpers::readLittleEndianSafe<uint64_t>(const std::vector<uint8_t>&, size_t, bool&);