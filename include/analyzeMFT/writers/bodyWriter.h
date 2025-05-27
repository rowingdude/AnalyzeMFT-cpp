#ifndef ANALYZEMFT_BODYWRITER_H
#define ANALYZEMFT_BODYWRITER_H

#include "fileWriter.h"

class BodyWriter : public FileWriter {
public:
    BodyWriter();
    
    bool write(const std::vector<const MftRecord*>& records, const std::string& outputFile) override;

protected:
    bool writeRecord(std::ostream& stream, const MftRecord* record) override;

private:
    std::string formatBodyEntry(const MftRecord* record, const WindowsTime& time, char macb) const;
    void writeTimeEntry(std::ostream& stream, const MftRecord* record, const WindowsTime& time, char macb);
};

#endif