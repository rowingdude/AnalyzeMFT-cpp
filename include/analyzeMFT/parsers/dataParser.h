#ifndef ANALYZEMFT_DATAPARSER_H
#define ANALYZEMFT_DATAPARSER_H

#include <vector>
#include <cstdint>
#include <string>

class DataParser {
public:
    struct DataAttribute {
        std::string name;
        bool nonResident;
        uint32_t contentSize;
        uint64_t startingVcn;
        uint64_t lastVcn;
        uint16_t dataRunsOffset;
        uint16_t compressionUnit;
        uint64_t allocatedSize;
        uint64_t actualSize;
        uint64_t initializedSize;
        std::vector<uint8_t> content;
        std::vector<DataRun> dataRuns;
        bool valid;
    };

    struct DataRun {
        uint64_t length;
        int64_t offset;
        bool sparse;
    };

    static bool parse(const std::vector<uint8_t>& data, size_t offset, DataAttribute& attr);
    static std::vector<DataRun> parseDataRuns(const std::vector<uint8_t>& data, size_t offset);
    static uint64_t calculateTotalClusters(const std::vector<DataRun>& dataRuns);

private:
    template<typename T>
    static T readLittleEndian(const std::vector<uint8_t>& data, size_t offset);
    
    static std::string readUtf16String(const std::vector<uint8_t>& data, size_t offset, size_t length);
    static int64_t readSignedInteger(const std::vector<uint8_t>& data, size_t offset, size_t length);
    static uint64_t readUnsignedInteger(const std::vector<uint8_t>& data, size_t offset, size_t length);
};

#endif