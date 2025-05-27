#include "csvWriter.h"
#include "../core/constants.h"
#include "../utils/stringUtils.h"
#include <fstream>
#include <iostream>

CsvWriter::CsvWriter(char delimiter, bool includeHeader) 
    : delimiter(delimiter), includeHeader(includeHeader) {
}

bool CsvWriter::write(const std::vector<const MftRecord*>& records, const std::string& outputFile) {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        return false;
    }
    
    try {
        if (includeHeader) {
            if (!writeHeader(file)) {
                return false;
            }
        }
        
        for (const auto* record : records) {
            if (!writeRecord(file, record)) {
                return false;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool CsvWriter::writeHeader(std::ostream& stream) {
    for (size_t i = 0; i < CSV_HEADER.size(); ++i) {
        writeField(stream, CSV_HEADER[i], i == CSV_HEADER.size() - 1);
    }
    stream << "\n";
    return stream.good();
}

bool CsvWriter::writeRecord(std::ostream& stream, const MftRecord* record) {
    if (!record) return false;
    
    std::vector<std::string> csvRow = record->toCsv();
    
    for (size_t i = 0; i < csvRow.size(); ++i) {
        writeField(stream, csvRow[i], i == csvRow.size() - 1);
    }
    stream << "\n";
    
    return stream.good();
}

void CsvWriter::writeField(std::ostream& stream, const std::string& field, bool isLast) {
    stream << escapeCsvField(field);
    if (!isLast) {
        stream << delimiter;
    }
}

std::string CsvWriter::escapeCsvField(const std::string& field) const {
    if (field.find(delimiter) == std::string::npos && 
        field.find('"') == std::string::npos && 
        field.find('\n') == std::string::npos &&
        field.find('\r') == std::string::npos) {
        return field;
    }
    
    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    return escaped;
}

void CsvWriter::setDelimiter(char delimiter) {
    this->delimiter = delimiter;
}

void CsvWriter::setIncludeHeader(bool include) {
    this->includeHeader = include;
}