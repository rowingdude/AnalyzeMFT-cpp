#include "indexParser.h"
#include "../utils/stringUtils.h"

template<typename T>
T IndexParser::readLittleEndian(const std::vector<uint8_t>& data, size_t offset) {
    if (offset + sizeof(T) > data.size()) {
        return T{};
    }
    
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(data[offset + i]) << (i * 8);
    }
    return value;
}

std::string IndexParser::readUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t length) {
    if (offset + length * 2 > data.size()) {
        return "";
    }
    
    std::wstring wstr;
    for (size_t i = 0; i < length; ++i) {
        uint16_t wchar = readLittleEndian<uint16_t>(data, offset + i * 2);
        wstr.push_back(static_cast<wchar_t>(wchar));
    }
    
    return StringUtils::wstringToString(wstr);
}

bool IndexParser::parseIndexRoot(const std::vector<uint8_t>& data, size_t offset, IndexRootAttribute& attr) {
    if (offset + 16 > data.size()) {
        attr.valid = false;
        return false;
    }
    
    try {
        attr.attributeType = readLittleEndian<uint32_t>(data, offset);
        attr.collationRule = readLittleEndian<uint32_t>(data, offset + 4);
        attr.indexAllocationSize = readLittleEndian<uint32_t>(data, offset + 8);
        attr.clustersPerIndexRecord = data[offset + 12];
        
        if (offset + 32 > data.size()) {
            attr.valid = false;
            return false;
        }
        
        attr.indexHeader.firstEntryOffset = readLittleEndian<uint32_t>(data, offset + 16);
        attr.indexHeader.totalSizeOfEntries = readLittleEndian<uint32_t>(data, offset + 20);
        attr.indexHeader.allocatedSizeOfEntries = readLittleEndian<uint32_t>(data, offset + 24);
        attr.indexHeader.flags = data[offset + 28];
        
        size_t entriesOffset = offset + 16 + attr.indexHeader.firstEntryOffset;
        attr.entries = parseIndexEntries(data, entriesOffset, attr.indexHeader);
        
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

bool IndexParser::parseIndexAllocation(const std::vector<uint8_t>& data, size_t offset, IndexAllocationAttribute& attr) {
    if (offset + 24 > data.size()) {
        attr.valid = false;
        return false;
    }
    
    try {
        attr.startingVcn = readLittleEndian<uint64_t>(data, offset + 16);
        attr.lastVcn = readLittleEndian<uint64_t>(data, offset + 24);
        attr.dataRunsOffset = readLittleEndian<uint16_t>(data, offset + 32);
        
        if (attr.dataRunsOffset > 0 && offset + attr.dataRunsOffset < data.size()) {
            size_t runStart = offset + attr.dataRunsOffset;
            size_t runEnd = std::min(data.size(), offset + readLittleEndian<uint32_t>(data, offset + 4));
            attr.dataRuns.assign(data.begin() + runStart, data.begin() + runEnd);
        }
        
        attr.valid = true;
        return true;
    } catch (const std::exception& e) {
        attr.valid = false;
        return false;
    }
}

std::vector<IndexParser::IndexEntry> IndexParser::parseIndexEntries(const std::vector<uint8_t>& data, size_t offset, const IndexHeader& header) {
    std::vector<IndexEntry> entries;
    size_t currentOffset = offset;
    size_t endOffset = offset + header.totalSizeOfEntries;
    
    while (currentOffset < endOffset && currentOffset + 16 <= data.size()) {
        IndexEntry entry;
        
        entry.fileReference = readLittleEndian<uint64_t>(data, currentOffset);
        entry.length = readLittleEndian<uint16_t>(data, currentOffset + 8);
        entry.attributeLength = readLittleEndian<uint16_t>(data, currentOffset + 10);
        entry.flags = readLittleEndian<uint32_t>(data, currentOffset + 12);
        
        entry.isLastEntry = (entry.flags & INDEX_ENTRY_END) != 0;
        entry.hasSubNode = (entry.flags & INDEX_ENTRY_NODE) != 0;
        
        if (entry.length == 0 || currentOffset + entry.length > data.size()) {
            break;
        }
        
        if (entry.attributeLength > 0 && currentOffset + 16 + entry.attributeLength <= data.size()) {
            entry.attribute.assign(data.begin() + currentOffset + 16,
                                 data.begin() + currentOffset + 16 + entry.attributeLength);
            
            if (entry.attributeLength >= 66) {
                uint8_t nameLength = entry.attribute[64];
                if (nameLength > 0 && 66 + nameLength * 2 <= entry.attribute.size()) {
                    entry.filename = readUtf16String(entry.attribute, 66, nameLength);
                }
            }
        }
        
        if (entry.hasSubNode && currentOffset + entry.length - 8 < data.size()) {
            entry.subNodeVcn = readLittleEndian<uint64_t>(data, currentOffset + entry.length - 8);
        }
        
        entries.push_back(entry);
        
        if (entry.isLastEntry) {
            break;
        }
        
        currentOffset += entry.length;
    }
    
    return entries;
}

std::string IndexParser::getCollationRuleString(uint32_t rule) {
    switch (rule) {
        case COLLATION_BINARY: return "BINARY";
        case COLLATION_FILENAME: return "FILENAME";
        case COLLATION_UNICODE_STRING: return "UNICODE_STRING";
        case COLLATION_NTOFS_ULONG: return "NTOFS_ULONG";
        case COLLATION_NTOFS_SID: return "NTOFS_SID";
        case COLLATION_NTOFS_SECURITY_HASH: return "NTOFS_SECURITY_HASH";
        case COLLATION_NTOFS_ULONGS: return "NTOFS_ULONGS";
        default: return "Unknown (" + std::to_string(rule) + ")";
    }
}