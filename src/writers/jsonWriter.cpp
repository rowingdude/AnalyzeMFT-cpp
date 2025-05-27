#include "jsonWriter.h"
#include "../utils/stringUtils.h"
#include <fstream>
#include <iomanip>

JsonWriter::JsonWriter(bool prettyPrint) 
    : prettyPrint(prettyPrint), indentLevel(0), firstRecord(true) {
}

bool JsonWriter::write(const std::vector<const MftRecord*>& records, const std::string& outputFile) {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        return false;
    }
    
    try {
        firstRecord = true;
        indentLevel = 0;
        
        if (!writeHeader(file)) {
            return false;
        }
        
        for (const auto* record : records) {
            if (!writeRecord(file, record)) {
                return false;
            }
        }
        
        if (!writeFooter(file)) {
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool JsonWriter::writeHeader(std::ostream& stream) {
    stream << "[";
    if (prettyPrint) {
        stream << "\n";
        indentLevel++;
    }
    return stream.good();
}

bool JsonWriter::writeRecord(std::ostream& stream, const MftRecord* record) {
    if (!record) return false;
    
    if (!firstRecord) {
        stream << ",";
        if (prettyPrint) {
            stream << "\n";
        }
    }
    firstRecord = false;
    
    if (prettyPrint) {
        stream << getIndent();
    }
    
    writeJsonObject(stream, record);
    return stream.good();
}

bool JsonWriter::writeFooter(std::ostream& stream) {
    if (prettyPrint) {
        stream << "\n";
        indentLevel--;
        stream << getIndent();
    }
    stream << "]";
    return stream.good();
}

void JsonWriter::writeJsonObject(std::ostream& stream, const MftRecord* record) {
    stream << "{";
    if (prettyPrint) {
        stream << "\n";
        indentLevel++;
    }
    
    bool first = true;
    auto writeField = [&](const std::string& key, const std::string& value) {
        if (!first) {
            stream << ",";
            if (prettyPrint) stream << "\n";
        }
        first = false;
        
        if (prettyPrint) {
            stream << getIndent();
        }
        writeJsonField(stream, key, value);
    };
    
    writeField("recordNumber", std::to_string(record->recordnum));
    writeField("filename", record->filename);
    writeField("filesize", std::to_string(record->filesize));
    writeField("sequenceNumber", std::to_string(record->seq));
    writeField("parentRecordNumber", std::to_string(record->getParentRecordNum()));
    writeField("flags", std::to_string(record->flags));
    writeField("fileType", record->getFileType());
    
    writeField("siCreationTime", record->siTimes.crtime.getDateTimeString());
    writeField("siModificationTime", record->siTimes.mtime.getDateTimeString());
    writeField("siAccessTime", record->siTimes.atime.getDateTimeString());
    writeField("siEntryTime", record->siTimes.ctime.getDateTimeString());
    
    writeField("fnCreationTime", record->fnTimes.crtime.getDateTimeString());
    writeField("fnModificationTime", record->fnTimes.mtime.getDateTimeString());
    writeField("fnAccessTime", record->fnTimes.atime.getDateTimeString());
    writeField("fnEntryTime", record->fnTimes.ctime.getDateTimeString());
    
    if (!record->objectId.empty()) {
        writeField("objectId", record->objectId);
        writeField("birthVolumeId", record->birthVolumeId);
        writeField("birthObjectId", record->birthObjectId);
        writeField("birthDomainId", record->birthDomainId);
    }
    
    if (!record->md5.empty()) {
        writeField("md5", record->md5);
        writeField("sha256", record->sha256);
        writeField("sha512", record->sha512);
        writeField("crc32", record->crc32, true);
    }
    
    if (prettyPrint) {
        stream << "\n";
        indentLevel--;
        stream << getIndent();
    }
    stream << "}";
}

void JsonWriter::writeJsonField(std::ostream& stream, const std::string& key, const std::string& value, bool isLast) {
    stream << "\"" << escapeJsonString(key) << "\": \"" << escapeJsonString(value) << "\"";
}

std::string JsonWriter::escapeJsonString(const std::string& str) const {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (c < 32) {
                    escaped += "\\u" + StringUtils::toUpper(std::to_string(static_cast<int>(c)));
                } else {
                    escaped += c;
                }
                break;
        }
    }
    return escaped;
}

std::string JsonWriter::getIndent() const {
    return std::string(indentLevel * 2, ' ');
}

void JsonWriter::setPrettyPrint(bool pretty) {
    this->prettyPrint = pretty;
}