#ifndef ANALYZEMFT_EXCELWRITER_H
#define ANALYZEMFT_EXCELWRITER_H

#include "fileWriter.h"

class ExcelWriter : public FileWriter {
public:
    ExcelWriter();
    ~ExcelWriter();
    
    bool write(const std::vector<const MftRecord*>& records, const std::string& outputFile) override;

protected:
    bool writeRecord(std::ostream& stream, const MftRecord* record) override;

private:
    struct ExcelImpl;
    std::unique_ptr<ExcelImpl> impl;
    
    bool initializeWorkbook();
    bool finalizeWorkbook();
    bool writeExcelHeader();
    bool writeExcelRecord(const MftRecord* record, int row);
};

#endif