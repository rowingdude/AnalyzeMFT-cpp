#ifndef ANALYZEMFT_FILEWRITER_H
#define ANALYZEMFT_FILEWRITER_H

#include <string>
#include <vector>
#include "../core/mftRecord.h"

class FileWriter {
public:
    virtual ~FileWriter() = default;
    virtual bool write(const std::vector<const MftRecord*>& records, const std::string& outputFile) = 0;
    
protected:
    virtual bool writeHeader(std::ostream& stream) { return true; }
    virtual bool writeRecord(std::ostream& stream, const MftRecord* record) = 0;
    virtual bool writeFooter(std::ostream& stream) { return true; }
    
    std::string escapeString(const std::string& str, const std::string& chars = "\"") const;
    std::string formatTimestamp(const WindowsTime& time) const;
};

#endif