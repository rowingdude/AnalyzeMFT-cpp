#ifndef ANALYZEMFT_JSONWRITER_H
#define ANALYZEMFT_JSONWRITER_H

#include "fileWriter.h"
#include <memory>

class JsonWriter : public FileWriter {
public:
    JsonWriter(bool prettyPrint = true);
    
    bool write(const std::vector<const MftRecord*>& records, const std::string& outputFile) override;
    
    void setPrettyPrint(bool pretty);

protected:
    bool writeHeader(std::ostream& stream) override;
    bool writeRecord(std::ostream& stream, const MftRecord* record) override;
    bool writeFooter(std::ostream& stream) override;

private:
    bool prettyPrint;
    int indentLevel;
    bool firstRecord;
    
    std::string escapeJsonString(const std::string& str) const;
    std::string getIndent() const;
    void writeJsonField(std::ostream& stream, const std::string& key, const std::string& value, bool isLast = false);
    void writeJsonObject(std::ostream& stream, const MftRecord* record);
};

#endif