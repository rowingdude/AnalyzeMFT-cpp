#ifndef ANALYZEMFT_FILENAMEPARSER_H
#define ANALYZEMFT_FILENAMEPARSER_H

#include <vector>
#include <cstdint>
#include <string>
#include "../core/winTime.h"

class FilenameParser {
public:
    struct FilenameAttribute {
        uint64_t parentDirectory;
        WindowsTime creationTime;
        WindowsTime modificationTime;
        WindowsTime accessTime;
        WindowsTime entryTime;
        uint64_t allocatedSize;
        uint64_t realSize;
        uint32_t flags;
        uint32_t reparseValue;
        uint8_t filenameLength;
        uint8_t filenameNamespace;
        std::string filename;
        bool valid;
    };

    enum FilenameNamespace {
        POSIX = 0,
        WIN32 = 1,
        DOS = 2,
        WIN32_AND_DOS = 3
    };

    static bool parse(const std::vector<uint8_t>& data, size_t offset, FilenameAttribute& attr);
    static std::string getNamespaceString(uint8_t namespaceValue);
    static uint64_t getParentRecordNumber(uint64_t parentReference);
    static uint16_t getParentSequenceNumber(uint64_t parentReference);

private:
    template<typename T>
    static T readLittleEndian(const std::vector<uint8_t>& data, size_t offset);
    
    static std::string readUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t length);
};

#endif