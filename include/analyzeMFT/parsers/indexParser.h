#ifndef ANALYZEMFT_INDEXPARSER_H
#define ANALYZEMFT_INDEXPARSER_H

#include <vector>
#include <cstdint>
#include <string>

class IndexParser {
public:
    struct IndexRootAttribute {
        uint32_t attributeType;
        uint32_t collationRule;
        uint32_t indexAllocationSize;
        uint8_t clustersPerIndexRecord;
        uint8_t padding[3];
        IndexHeader indexHeader;
        std::vector<IndexEntry> entries;
        bool valid;
    };

    struct IndexAllocationAttribute {
        uint64_t startingVcn;
        uint64_t lastVcn;
        uint16_t dataRunsOffset;
        std::vector<uint8_t> dataRuns;
        bool valid;
    };

    struct IndexHeader {
        uint32_t firstEntryOffset;
        uint32_t totalSizeOfEntries;
        uint32_t allocatedSizeOfEntries;
        uint8_t flags;
        uint8_t padding[3];
    };

    struct IndexEntry {
        uint64_t fileReference;
        uint16_t length;
        uint16_t attributeLength;
        uint32_t flags;
        std::vector<uint8_t> attribute;
        uint64_t subNodeVcn;
        std::string filename;
        bool isLastEntry;
        bool hasSubNode;
    };

    enum CollationRule {
        COLLATION_BINARY = 0x00,
        COLLATION_FILENAME = 0x01,
        COLLATION_UNICODE_STRING = 0x02,
        COLLATION_NTOFS_ULONG = 0x10,
        COLLATION_NTOFS_SID = 0x11,
        COLLATION_NTOFS_SECURITY_HASH = 0x12,
        COLLATION_NTOFS_ULONGS = 0x13
    };

    static bool parseIndexRoot(const std::vector<uint8_t>& data, size_t offset, IndexRootAttribute& attr);
    static bool parseIndexAllocation(const std::vector<uint8_t>& data, size_t offset, IndexAllocationAttribute& attr);
    static std::vector<IndexEntry> parseIndexEntries(const std::vector<uint8_t>& data, size_t offset, const IndexHeader& header);
    static std::string getCollationRuleString(uint32_t rule);

private:
    static const uint32_t INDEX_ENTRY_NODE = 0x01;
    static const uint32_t INDEX_ENTRY_END = 0x02;

    template<typename T>
    static T readLittleEndian(const std::vector<uint8_t>& data, size_t offset);
    
    static std::string readUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t length);
};

#endif