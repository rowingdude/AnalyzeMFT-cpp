#ifndef ANALYZEMFT_MFTRECORD_H
#define ANALYZEMFT_MFTRECORD_H

#include <vector>
#include <string>
#include <unordered_set>
#include <memory>
#include <cstdint>
#include "winTime.h"
#include "constants.h"

struct AttributeListEntry {
    uint32_t type;
    std::string name;
    uint64_t vcn;
    uint64_t reference;
};

struct SecurityDescriptor {
    uint8_t revision;
    uint16_t control;
    uint32_t ownerOffset;
    uint32_t groupOffset;
    uint32_t saclOffset;
    uint32_t daclOffset;
};

struct VolumeInfo {
    uint8_t majorVersion;
    uint8_t minorVersion;
    uint16_t flags;
};

struct DataAttribute {
    std::string name;
    bool nonResident;
    uint32_t contentSize;
    uint64_t startVcn;
    uint64_t lastVcn;
};

struct IndexRoot {
    uint32_t attrType;
    uint32_t collationRule;
    uint32_t indexAllocSize;
    uint8_t clustersPerIndex;
};

struct IndexAllocation {
    uint16_t dataRunsOffset;
};

struct BitmapAttribute {
    uint32_t size;
    std::vector<uint8_t> data;
};

struct ReparsePoint {
    uint32_t reparseTag;
    uint16_t dataLength;
    std::vector<uint8_t> data;
};

struct EaInformation {
    uint32_t eaSize;
    uint32_t eaCount;
};

struct ExtendedAttribute {
    uint32_t nextEntryOffset;
    uint8_t flags;
    std::string name;
    std::vector<uint8_t> value;
};

struct LoggedUtilityStream {
    uint64_t size;
    std::vector<uint8_t> data;
};

class MftRecord {
public:
    MftRecord(const std::vector<uint8_t>& rawRecord, bool computeHashes = false, int debugLevel = 0);
    
    std::vector<std::string> toCsv() const;
    void computeHashes();
    std::string getFileType() const;
    uint64_t getParentRecordNum() const;
    
    uint32_t magic;
    uint16_t updOff;
    uint16_t updCnt;
    uint64_t lsn;
    uint16_t seq;
    uint16_t link;
    uint16_t attrOff;
    uint16_t flags;
    uint32_t size;
    uint32_t allocSizef;
    uint64_t baseRef;
    uint16_t nextAttrid;
    uint32_t recordnum;
    std::string filename;
    
    struct {
        WindowsTime crtime;
        WindowsTime mtime;
        WindowsTime atime;
        WindowsTime ctime;
    } siTimes;
    
    struct {
        WindowsTime crtime;
        WindowsTime mtime;
        WindowsTime atime;
        WindowsTime ctime;
    } fnTimes;
    
    uint64_t filesize;
    std::unordered_set<uint32_t> attributeTypes;
    std::vector<AttributeListEntry> attributeList;
    std::string objectId;
    std::string birthVolumeId;
    std::string birthObjectId;
    std::string birthDomainId;
    uint64_t parentRef;
    
    std::string md5;
    std::string sha256;
    std::string sha512;
    std::string crc32;
    
    std::unique_ptr<SecurityDescriptor> securityDescriptor;
    std::string volumeName;
    std::unique_ptr<VolumeInfo> volumeInfo;
    std::unique_ptr<DataAttribute> dataAttribute;
    std::unique_ptr<IndexRoot> indexRoot;
    std::unique_ptr<IndexAllocation> indexAllocation;
    std::unique_ptr<BitmapAttribute> bitmap;
    std::unique_ptr<ReparsePoint> reparsePoint;
    std::unique_ptr<EaInformation> eaInformation;
    std::unique_ptr<ExtendedAttribute> ea;
    std::unique_ptr<LoggedUtilityStream> loggedUtilityStream;

private:
    std::vector<uint8_t> rawRecord;
    std::unique_ptr<MftAttributeValidator> validator;
    
    int debugLevel;
    bool computeHashesFlag;
    bool applyFixupArray();
    bool validateFixupArray() const;
    void log(const std::string& message, int level) const;
    void parseRecord();
    void parseAttributes();
    void parseSiAttribute(size_t offset);
    void parseFnAttribute(size_t offset);
    void parseObjectIdAttribute(size_t offset);
    void parseAttributeList(size_t offset);
    void parseSecurityDescriptor(size_t offset);
    void parseVolumeName(size_t offset);
    void parseVolumeInformation(size_t offset);
    void parseData(size_t offset);
    void parseIndexRoot(size_t offset);
    void parseIndexAllocation(size_t offset);
    void parseBitmap(size_t offset);
    void parseReparsePoint(size_t offset);
    void parseEaInformation(size_t offset);
    void parseEa(size_t offset);  
    void parseLoggedUtilityStream(size_t offset);
    bool parseSiAttributeWithValidation(size_t offset);
    bool parseFnAttributeWithValidation(size_t offset);
    bool parseAttributeListWithValidation(size_t offset);
    bool parseObjectIdAttributeWithValidation(size_t offset);
    bool parseSecurityDescriptorWithValidation(size_t offset);
    bool parseVolumeNameWithValidation(size_t offset);
    bool parseVolumeInformationWithValidation(size_t offset);
    bool parseDataWithValidation(size_t offset);
    bool parseIndexRootWithValidation(size_t offset);
    bool parseIndexAllocationWithValidation(size_t offset);
    bool parseBitmapWithValidation(size_t offset);
    bool parseReparsePointWithValidation(size_t offset);
    bool parseEaInformationWithValidation(size_t offset);
    bool parseEaWithValidation(size_t offset);
    bool parseLoggedUtilityStreamWithValidation(size_t offset);
    template<typename T>
    T readLittleEndian(size_t offset) const;
    
    std::string readUtf16String(size_t offset, size_t length) const;
    std::string bytesToGuid(const uint8_t* bytes) const;
};

#endif