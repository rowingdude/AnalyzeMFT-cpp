#include "fileWriter.h"
#include <algorithm>

std::string FileWriter::escapeString(const std::string& str, const std::string& chars) const {
    std::string escaped = str;
    for (char c : chars) {
        size_t pos = 0;
        std::string target(1, c);
        std::string replacement = "\\" + target;
        while ((pos = escaped.find(target, pos)) != std::string::npos) {
            escaped.replace(pos, 1, replacement);
            pos += replacement.length();
        }
    }
    return escaped;
}

std::string FileWriter::formatTimestamp(const WindowsTime& time) const {
    return time.getDateTimeString();
}