#include "timelineWriter.h"
#include <fstream>

TimelineWriter::TimelineWriter() {
}

bool TimelineWriter::write(const std::vector<const MftRecord*>& records, const std::string& outputFile) {
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

bool TimelineWriter::writeRecord(std::ostream& stream, const MftRecord* record) {
    if (!record) return false;
    
    writeTimelineEvent(stream, record, record->fnTimes.crtime, "CREATE");
    writeTimelineEvent(stream, record, record->fnTimes.mtime, "MODIFY");
    writeTimelineEvent(stream, record, record->fnTimes.atime, "ACCESS");
    writeTimelineEvent(stream, record, record->fnTimes.ctime, "CHANGE");
    
    return stream.good();
}

void TimelineWriter::writeTimelineEvent(std::ostream& stream, const MftRecord* record, const WindowsTime& time, const std::string& eventType) {
   if (!time.isValid()) return;
   
   std::string entry = formatTimelineEntry(record, time, eventType);
   stream << entry << "\n";
}

std::string TimelineWriter::formatTimelineEntry(const MftRecord* record, const WindowsTime& time, const std::string& eventType) const {
   std::string entry;
   entry += std::to_string(time.getUnixTime());
   entry += "|MFT|";
   entry += eventType;
   entry += "|||||";
   entry += record->filename;
   entry += "|";
   entry += std::to_string(record->recordnum);
   entry += "||||";
   
   return entry;
}