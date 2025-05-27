#ifndef ANALYZEMFT_CSVWRITER_H
#define ANALYZEMFT_CSVWRITER_H

#include "fileWriter.h"
#include <fstream>
#include <memory>

class CsvWriter : public FileWriter {
public:
    CsvWriter(char delimiter = ',', bool includeHeader = true);
    
    bool write(const std::vector<const MftRecord*>& records, const std::string& outputFile) override;
    
    void setDelimiter(char delimiter);
    void setIncludeHeader(bool include);

protected:
    bool writeHeader(std::ostream& stream) override;
    bool writeRecord(std::ostream& stream, const MftRecord* record) override;

private:
    char delimiter;
    bool includeHeader;
    
    std::string escapeCsvField(const std::string& field) const;
    void writeField(std::ostream& stream, const std::string& field, bool isLast = false);
};

#endif