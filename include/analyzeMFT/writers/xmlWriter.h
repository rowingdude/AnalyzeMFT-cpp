#ifndef ANALYZEMFT_XMLWRITER_H
#define ANALYZEMFT_XMLWRITER_H

#include "fileWriter.h"

class XmlWriter : public FileWriter {
public:
    XmlWriter(bool prettyPrint = true);
    
    bool write(const std::vector<const MftRecord*>& records, const std::string& outputFile) override;
    
    void setPrettyPrint(bool pretty);

protected:
    bool writeHeader(std::ostream& stream) override;
    bool writeRecord(std::ostream& stream, const MftRecord* record) override;
    bool writeFooter(std::ostream& stream) override;

private:
    bool prettyPrint;
    int indentLevel;
    
    std::string escapeXmlString(const std::string& str) const;
    std::string getIndent() const;
    void writeXmlElement(std::ostream& stream, const std::string& tag, const std::string& value);
    void writeRecordElement(std::ostream& stream, const MftRecord* record);
};

#endif