#include "../../include/analyzeMFT/writers/sqliteWriter.h"

#ifdef HAVE_SQLITE3
#include <sqlite3.h>
#endif

#include "../core/constants.h"
#include "../utils/fsUtils.h"
#include <iostream>
#include <fstream>

SqliteWriter::SqliteWriter() : database(nullptr), insertStatement(nullptr) {
}

SqliteWriter::~SqliteWriter() {
    closeDatabase();
}

bool SqliteWriter::write(const std::vector<const MftRecord*>& records, const std::string& outputFile) {
    if (!openDatabase(outputFile)) {
        return false;
    }
    
    if (!createTables()) {
        return false;
    }
    
    if (!prepareStatements()) {
        return false;
    }
    
    sqlite3_exec(database, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
    
    for (const auto* record : records) {
        if (!insertRecord(record)) {
            sqlite3_exec(database, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
    }
    
    sqlite3_exec(database, "COMMIT", nullptr, nullptr, nullptr);
    return true;
}

bool SqliteWriter::writeRecord(std::ostream& stream, const MftRecord* record) {
    return true;
}

bool SqliteWriter::openDatabase(const std::string& filename) {
    int result = sqlite3_open(filename.c_str(), &database);
    if (result != SQLITE_OK) {
        return false;
    }
    return true;
}

bool SqliteWriter::createTables() {
    if (!executeSqlScript("attributeTypes.sql")) {
        return false;
    }
    
    if (!executeSqlScript("fileRecordFlags.sql")) {
        return false;
    }
    
    if (!executeSqlScript("schema.sql")) {
        return false;
    }
    
    const char* createMftRecords = R"(
        CREATE TABLE IF NOT EXISTS mft_records (
            record_number INTEGER PRIMARY KEY,
            filename TEXT,
            parent_record_number INTEGER,
            file_size INTEGER,
            is_directory INTEGER,
            creation_time TEXT,
            modification_time TEXT,
            access_time TEXT,
            entry_time TEXT,
            attribute_types TEXT,
            flags INTEGER,
            sequence_number INTEGER,
            object_id TEXT,
            birth_volume_id TEXT,
            birth_object_id TEXT,
            birth_domain_id TEXT,
            md5 TEXT,
            sha256 TEXT,
            sha512 TEXT,
            crc32 TEXT
        )
    )";
    
    char* errorMsg = nullptr;
    int result = sqlite3_exec(database, createMftRecords, nullptr, nullptr, &errorMsg);
    if (result != SQLITE_OK) {
        if (errorMsg) {
            sqlite3_free(errorMsg);
        }
        return false;
    }
    
    return true;
}

bool SqliteWriter::prepareStatements() {
    const char* insertSql = R"(
        INSERT INTO mft_records (
            record_number, filename, parent_record_number, file_size,
            is_directory, creation_time, modification_time, access_time,
            entry_time, attribute_types, flags, sequence_number,
            object_id, birth_volume_id, birth_object_id, birth_domain_id,
            md5, sha256, sha512, crc32
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    int result = sqlite3_prepare_v2(database, insertSql, -1, &insertStatement, nullptr);
    return result == SQLITE_OK;
}

bool SqliteWriter::insertRecord(const MftRecord* record) {
    if (!insertStatement || !record) {
        return false;
    }
    
    sqlite3_reset(insertStatement);
    
    sqlite3_bind_int(insertStatement, 1, record->recordnum);
    sqlite3_bind_text(insertStatement, 2, record->filename.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(insertStatement, 3, record->getParentRecordNum());
    sqlite3_bind_int64(insertStatement, 4, record->filesize);
    sqlite3_bind_int(insertStatement, 5, (record->flags & FILE_RECORD_IS_DIRECTORY) ? 1 : 0);
    sqlite3_bind_text(insertStatement, 6, record->fnTimes.crtime.getDateTimeString().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStatement, 7, record->fnTimes.mtime.getDateTimeString().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStatement, 8, record->fnTimes.atime.getDateTimeString().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStatement, 9, record->fnTimes.ctime.getDateTimeString().c_str(), -1, SQLITE_STATIC);
    
    std::string attributeTypes;
    for (auto attr : record->attributeTypes) {
        if (!attributeTypes.empty()) attributeTypes += ",";
        attributeTypes += std::to_string(attr);
    }
    sqlite3_bind_text(insertStatement, 10, attributeTypes.c_str(), -1, SQLITE_STATIC);
    
    sqlite3_bind_int(insertStatement, 11, record->flags);
    sqlite3_bind_int(insertStatement, 12, record->seq);
    sqlite3_bind_text(insertStatement, 13, record->objectId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStatement, 14, record->birthVolumeId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStatement, 15, record->birthObjectId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStatement, 16, record->birthDomainId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStatement, 17, record->md5.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStatement, 18, record->sha256.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStatement, 19, record->sha512.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insertStatement, 20, record->crc32.c_str(), -1, SQLITE_STATIC);
    
    int result = sqlite3_step(insertStatement);
    return result == SQLITE_DONE;
}

bool SqliteWriter::executeSqlScript(const std::string& scriptName) {
    std::string scriptPath = getSqlScriptPath(scriptName);
    
    std::ifstream file(scriptPath);
    if (!file.is_open()) {
        return true;
    }
    
    std::string sql((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    
    char* errorMsg = nullptr;
    int result = sqlite3_exec(database, sql.c_str(), nullptr, nullptr, &errorMsg);
    if (result != SQLITE_OK) {
        if (errorMsg) {
            sqlite3_free(errorMsg);
        }
        return false;
    }
    
    return true;
}

std::string SqliteWriter::getSqlScriptPath(const std::string& scriptName) const {
    return FileSystemUtils::joinPath("data/sql", scriptName);
}

void SqliteWriter::closeDatabase() {
    if (insertStatement) {
        sqlite3_finalize(insertStatement);
        insertStatement = nullptr;
    }
    
    if (database) {
        sqlite3_close(database);
        database = nullptr;
    }
}