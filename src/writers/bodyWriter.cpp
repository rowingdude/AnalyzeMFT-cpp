#include "bodyWriter.h"
#include <fstream>

BodyWriter::BodyWriter() {
}

bool BodyWriter::write(const std::vector<const MftRecord*>& records, const std::string& outputFile) {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        return false;
    }
    
    try {
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

bool BodyWriter::writeRecord(std::ostream& stream, const MftRecord* record) {
    if (!record) return false;
    
    writeTimeEntry(stream, record, record->fnTimes.mtime, 'M');
    writeTimeEntry(stream, record, record->fnTimes.atime, 'A');
    writeTimeEntry(stream, record, record->fnTimes.ctime, 'C');
    writeTimeEntry(stream, record, record->fnTimes.crtime, 'B');
    
    return stream.good();
}

void BodyWriter::writeTimeEntry(std::ostream& stream, const MftRecord* record, const WindowsTime& time, char macb) {
    if (!time.isValid()) return;
    
    std::string entry = formatBodyEntry(record, time, macb);
    stream << entry << "\n";
}

std::string BodyWriter::formatBodyEntry(const MftRecord* record, const WindowsTime& time, char macb) const {
    std::string entry;
    entry += record->md5.empty() ? "0" : record->md5;
    entry += "|";
    entry += record->filename;
    entry += "|";
    entry += std::to_string(record->recordnum);
    entry += "|";
    entry += std::to_string(record->flags);
    entry += "|0|0|";
    entry += std::to_string(record->filesize);
    entry += "|";
    entry += std::to_string(time.getUnixTime());
    entry += "|";
    entry += std::to_string(time.getUnixTime());
    entry += "|";
    entry += std::to_string(time.getUnixTime());
    entry += "|";
    entry += std::to_string(time.getUnixTime());
    
    return entry;
}