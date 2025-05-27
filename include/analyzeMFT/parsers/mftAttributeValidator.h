#ifndef ANALYZEMFT_MFTATTRIBUTEVALIDATOR_H
#define ANALYZEMFT_MFTATTRIBUTEVALIDATOR_H

#include <vector>
#include <cstdint>
#include <string>
#include "validationHelpers.h"
#include "../core/winTime.h"

// Forward declarations for MFT record structures
struct SecurityDescriptor;
struct VolumeInfo;
struct DataAttribute;
struct IndexRoot;
struct IndexAllocation;
struct BitmapAttribute;
struct ReparsePoint;
struct EaInformation;
struct ExtendedAttribute;
struct LoggedUtilityStream;
struct AttributeListEntry;

class MftAttributeValidator {
public:
    MftAttributeValidator(int debugLevel = 0, uint32_t recordNumber = 0);
    
    ValidationHelpers::ValidationResult validateStandardInformation(const std::vector<uint8_t>& data, size_t offset, 
        WindowsTime& crtime, WindowsTime& mtime, WindowsTime& atime, WindowsTime& ctime);
    
    ValidationHelpers::ValidationResult validateFileName(const std::vector<uint8_t>& data, size_t offset,
        std::string& filename, WindowsTime& crtime, WindowsTime& mtime, WindowsTime& atime, WindowsTime& ctime,
        uint64_t& filesize, uint64_t& parentRef);
    
    ValidationHelpers::ValidationResult validateObjectId(const std::vector<uint8_t>& data, size_t offset,
        std::string& objectId, std::string& birthVolumeId, std::string& birthObjectId, std::string& birthDomainId);
    
    ValidationHelpers::ValidationResult validateAttributeList(const std::vector<uint8_t>& data, size_t offset,
        std::vector<AttributeListEntry>& attributeList);
    
    ValidationHelpers::ValidationResult validateSecurityDescriptor(const std::vector<uint8_t>& data, size_t offset,
        std::unique_ptr<SecurityDescriptor>& securityDescriptor);
    
    ValidationHelpers::ValidationResult validateVolumeName(const std::vector<uint8_t>& data, size_t offset,
        std::string& volumeName);
    
    ValidationHelpers::ValidationResult validateVolumeInformation(const std::vector<uint8_t>& data, size_t offset,
        std::unique_ptr<VolumeInfo>& volumeInfo);
    
    ValidationHelpers::ValidationResult validateData(const std::vector<uint8_t>& data, size_t offset,
        std::unique_ptr<DataAttribute>& dataAttribute);
    
    ValidationHelpers::ValidationResult validateIndexRoot(const std::vector<uint8_t>& data, size_t offset,
        std::unique_ptr<IndexRoot>& indexRoot);
    
    ValidationHelpers::ValidationResult validateIndexAllocation(const std::vector<uint8_t>& data, size_t offset,
        std::unique_ptr<IndexAllocation>& indexAllocation);
    
    ValidationHelpers::ValidationResult validateBitmap(const std::vector<uint8_t>& data, size_t offset,
        std::unique_ptr<BitmapAttribute>& bitmap);
    
    ValidationHelpers::ValidationResult validateReparsePoint(const std::vector<uint8_t>& data, size_t offset,
        std::unique_ptr<ReparsePoint>& reparsePoint);
    
    ValidationHelpers::ValidationResult validateEaInformation(const std::vector<uint8_t>& data, size_t offset,
        std::unique_ptr<EaInformation>& eaInformation);
    
    ValidationHelpers::ValidationResult validateEa(const std::vector<uint8_t>& data, size_t offset,
        std::unique_ptr<ExtendedAttribute>& ea);
    
    ValidationHelpers::ValidationResult validateLoggedUtilityStream(const std::vector<uint8_t>& data, size_t offset,
        std::unique_ptr<LoggedUtilityStream>& loggedUtilityStream);

    void setDebugLevel(int level) { debugLevel = level; }
    void setRecordNumber(uint32_t number) { recordNumber = number; }

private:
    int debugLevel;
    uint32_t recordNumber;
    
    void logValidationError(const std::string& message, int level = 1) const;
    void logValidationWarning(const std::string& message) const;
    void logValidationInfo(const std::string& message) const;
};

#endif