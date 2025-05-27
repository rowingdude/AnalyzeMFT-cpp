#ifndef ANALYZEMFT_SQLITEWRITER_H
#define ANALYZEMFT_SQLITEWRITER_H

#include "fileWriter.h"
#include <sqlite3.h>
#include <memory>

class SqliteWriter : public FileWriter {
public:
    SqliteWriter();
    ~SqliteWriter();
    
    bool write(const std::vector<const MftRecord*>& records, const std::string& outputFile) override;

protected:
    bool writeRecord(std::ostream& stream, const MftRecord* record) override;

private:
    sqlite3* database;
    sqlite3_stmt* insertStatement;
    
    bool openDatabase(const std::string& filename);
    bool createTables();
    bool prepareStatements();
    bool insertRecord(const MftRecord* record);
    bool executeSqlScript(const std::string& scriptPath);
    void closeDatabase();
    
    std::string getSqlScriptPath(const std::string& scriptName) const;
};

#endif