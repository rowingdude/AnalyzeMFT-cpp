#include "xmlWriter.h"
#include "../utils/stringUtils.h"
#include <fstream>

XmlWriter::XmlWriter(bool prettyPrint) 
    : prettyPrint(prettyPrint), indentLevel(0) {
}

bool XmlWriter::write(const std::vector<const MftRecord*>& records, const std::string& outputFile) {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        return false;
    }
    
    try {
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

bool XmlWriter::writeHeader(std::ostream& stream) {
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    if (prettyPrint) stream << "\n";
    
    stream << "<mft_records>";
    if (prettyPrint) {
        stream << "\n";
        indentLevel++;
    }
    
    return stream.good();
}

bool XmlWriter::writeRecord(std::ostream& stream, const MftRecord* record) {
    if (!record) return false;
    
    if (prettyPrint) {
        stream << getIndent();
    }
    
    writeRecordElement(stream, record);
    
    if (prettyPrint) {
        stream << "\n";
    }
    
    return stream.good();
}

bool XmlWriter::writeFooter(std::ostream& stream) {
    if (prettyPrint) {
        indentLevel--;
        stream << getIndent();
    }
    stream << "</mft_records>";
    return stream.good();
}

void XmlWriter::writeRecordElement(std::ostream& stream, const MftRecord* record) {
    stream << "<record>";
    if (prettyPrint) {
        stream << "\n";
        indentLevel++;
    }
    
    writeXmlElement(stream, "recordNumber", std::to_string(record->recordnum));
    writeXmlElement(stream, "filename", record->filename);
    writeXmlElement(stream, "filesize", std::to_string(record->filesize));
    writeXmlElement(stream, "sequenceNumber", std::to_string(record->seq));
    writeXmlElement(stream, "parentRecordNumber", std::to_string(record->getParentRecordNum()));
    writeXmlElement(stream, "flags", std::to_string(record->flags));
    writeXmlElement(stream, "fileType", record->getFileType());
    
    writeXmlElement(stream, "siCreationTime", record->siTimes.crtime.getDateTimeString());
    writeXmlElement(stream, "siModificationTime", record->siTimes.mtime.getDateTimeString());
    writeXmlElement(stream, "siAccessTime", record->siTimes.atime.getDateTimeString());
    writeXmlElement(stream, "siEntryTime", record->siTimes.ctime.getDateTimeString());
    
    writeXmlElement(stream, "fnCreationTime", record->fnTimes.crtime.getDateTimeString());
    writeXmlElement(stream, "fnModificationTime", record->fnTimes.mtime.getDateTimeString());
    writeXmlElement(stream, "fnAccessTime", record->fnTimes.atime.getDateTimeString());
    writeXmlElement(stream, "fnEntryTime", record->fnTimes.ctime.getDateTimeString());
    
    if (!record->objectId.empty()) {
        writeXmlElement(stream, "objectId", record->objectId);
        writeXmlElement(stream, "birthVolumeId", record->birthVolumeId);
        writeXmlElement(stream, "birthObjectId", record->birthObjectId);
        writeXmlElement(stream, "birthDomainId", record->birthDomainId);
    }
    
    if (!record->md5.empty()) {
        writeXmlElement(stream, "md5", record->md5);
        writeXmlElement(stream, "sha256", record->sha256);
        writeXmlElement(stream, "sha512", record->sha512);
        writeXmlElement(stream, "crc32", record->crc32);
    }
    
    if (prettyPrint) {
        indentLevel--;
        stream << getIndent();
    }
    stream << "</record>";
}

void XmlWriter::writeXmlElement(std::ostream& stream, const std::string& tag, const std::string& value) {
    if (prettyPrint) {
        stream << getIndent();
    }
    stream << "<" << tag << ">" << escapeXmlString(value) << "</" << tag << ">";
    if (prettyPrint) {
        stream << "\n";
    }
}

std::string XmlWriter::escapeXmlString(const std::string& str) const {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '&': escaped += "&amp;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&apos;"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

std::string XmlWriter::getIndent() const {
    return std::string(indentLevel * 2, ' ');
}

void XmlWriter::setPrettyPrint(bool pretty) {
    this->prettyPrint = pretty;
}