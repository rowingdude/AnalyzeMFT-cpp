#ifndef ANALYZEMFT_VALIDATIONHELPERS_H
#define ANALYZEMFT_VALIDATIONHELPERS_H

#include <vector>
#include <cstdint>
#include <string>

class ValidationHelpers {
public:
    struct ValidationResult {
        bool isValid;
        std::string errorMessage;
        size_t bytesConsumed;
        
        ValidationResult() : isValid(false), bytesConsumed(0) {}
        ValidationResult(bool valid, const std::string& error = "", size_t consumed = 0) 
            : isValid(valid), errorMessage(error), bytesConsumed(consumed) {}
    };

    struct AttributeHeader {
        uint32_t type;
        uint32_t length;
        uint8_t nonResident;
        uint8_t nameLength;
        uint16_t nameOffset;
        uint16_t flags;
        uint16_t attributeId;
        uint32_t valueLength;
        uint16_t valueOffset;
        uint8_t indexedFlag;
        uint8_t padding;
        bool valid;
    };

    struct NonResidentHeader {
        uint64_t startingVcn;
        uint64_t lastVcn;
        uint16_t dataRunsOffset;
        uint16_t compressionUnit;
        uint32_t padding;
        uint64_t allocatedSize;
        uint64_t actualSize;
        uint64_t initializedSize;
        bool valid;
    };

    static ValidationResult validateBounds(const std::vector<uint8_t>& data, size_t offset, size_t requiredSize);
    static ValidationResult validateAttributeHeader(const std::vector<uint8_t>& data, size_t offset, AttributeHeader& header);
    static ValidationResult validateNonResidentHeader(const std::vector<uint8_t>& data, size_t offset, NonResidentHeader& header);
    static ValidationResult validateUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t lengthInChars);
    static ValidationResult validateGuid(const std::vector<uint8_t>& data, size_t offset);
    static ValidationResult validateTimestamp(uint32_t low, uint32_t high);
    static ValidationResult validateFileReference(uint64_t reference);
    static ValidationResult validateSid(const std::vector<uint8_t>& data, size_t offset);
    static ValidationResult validateDataRuns(const std::vector<uint8_t>& data, size_t offset, size_t maxLength);

    template<typename T>
    static T readLittleEndianSafe(const std::vector<uint8_t>& data, size_t offset, bool& success);
    
    static std::string readUtf16StringSafe(const std::vector<uint8_t>& data, size_t offset, size_t lengthInChars, bool& success);
    static std::string bytesToGuidSafe(const std::vector<uint8_t>& data, size_t offset, bool& success);
    static bool isValidMftRecordNumber(uint64_t recordNumber);
    static bool isValidAttributeType(uint32_t attributeType);
    static std::string getValidationErrorMessage(const std::string& context, const std::string& error, size_t offset, uint32_t recordNumber);

    static const size_t MIN_ATTRIBUTE_SIZE = 16;
    static const size_t MAX_ATTRIBUTE_SIZE = 65536;
    static const size_t MAX_FILENAME_LENGTH = 255;
    static const size_t MAX_VOLUME_NAME_LENGTH = 128;
    static const uint32_t MAX_MFT_RECORD_NUMBER = 0x0000FFFFFFFFFFFF;
    static const uint16_t MAX_SEQUENCE_NUMBER = 0xFFFF;
};

#endif