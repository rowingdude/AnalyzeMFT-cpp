#include "excelWriter.h"
#include "../core/constants.h"
#include <fstream>

struct ExcelWriter::ExcelImpl {
    
    //TO DO: REVIEW libxlsxwriter API

    std::string outputFile;
    bool initialized = false;
};

ExcelWriter::ExcelWriter() : impl(std::make_unique<ExcelImpl>()) {
}

ExcelWriter::~ExcelWriter() = default;

bool ExcelWriter::write(const std::vector<const MftRecord*>& records, const std::string& outputFile) {
    impl->outputFile = outputFile;
    
    if (!initializeWorkbook()) {
        return false;
    }
    
    if (!writeExcelHeader()) {
        return false;
    }
    
    int row = 1;
    for (const auto* record : records) {
        if (!writeExcelRecord(record, row++)) {
            return false;
        }
    }
    
    return finalizeWorkbook();
}

bool ExcelWriter::writeRecord(std::ostream& stream, const MftRecord* record) {
    return true;
}

bool ExcelWriter::initializeWorkbook() {
    impl->initialized = true;
    return true;
}

bool ExcelWriter::writeExcelHeader() {
    return true;
}

bool ExcelWriter::writeExcelRecord(const MftRecord* record, int row) {
    return true;
}

bool ExcelWriter::finalizeWorkbook() {
    if (!impl->initialized) {
        return false;
    }
    
    std::ofstream file(impl->outputFile);
    if (!file.is_open()) {
        return false;
    }
    
    file << "Excel export not fully implemented. Use CSV format instead." << std::endl;
    return true;
}