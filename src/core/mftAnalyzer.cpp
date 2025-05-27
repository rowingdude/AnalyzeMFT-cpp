#include "mftAnalyzer.h"
#include "../writers/csvWriter.h"
#include "../writers/jsonWriter.h"
#include "../writers/xmlWriter.h"
#include "../writers/excelWriter.h"
#include "../writers/sqliteWriter.h"
#include "../writers/bodyWriter.h"
#include "../writers/timelineWriter.h"
#include "../utils/logger.h"
#include "constants.h"
#include <csignal>
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

MftAnalyzer* MftAnalyzer::currentInstance = nullptr;

MftAnalyzer::MftAnalyzer(const std::string& mftFile, const std::string& outputFile,
                        int debug, int verbosity, bool computeHashes, 
                        const std::string& exportFormat)
   : mftFile(mftFile), outputFile(outputFile), debug(debug), verbosity(verbosity),
     computeHashes(computeHashes), exportFormat(exportFormat) {
   
   currentInstance = this;
   setupInterruptHandler();
}

MftAnalyzer::~MftAnalyzer() {
   cleanup();
   currentInstance = nullptr;
}

void MftAnalyzer::setupInterruptHandler() {
   std::signal(SIGINT, signalHandler);
   std::signal(SIGTERM, signalHandler);
}

void MftAnalyzer::signalHandler(int signal) {
   if (currentInstance) {
       currentInstance->log("Interrupt received. Cleaning up...", 1);
       currentInstance->setInterruptFlag();
   }
}

void MftAnalyzer::log(const std::string& message, int level) const {
   if (level <= debug || level <= verbosity) {
       std::cout << message << std::endl;
   }
}

bool MftAnalyzer::analyze() {
   try {
       log("Starting MFT analysis...", 1);
       
       if (!initializeCsvWriter()) {
           log("Failed to initialize CSV writer", 0);
           return false;
       }
       
       if (!processMft()) {
           log("Failed to process MFT", 0);
           return false;
       }
       
       if (!writeOutput()) {
           log("Failed to write output", 0);
           return false;
       }
       
       return true;
   } catch (const std::exception& e) {
       log("An unexpected error occurred: " + std::string(e.what()), 0);
       return false;
   }
}

bool MftAnalyzer::processMft() {
   log("Processing MFT file: " + mftFile, 1);
   
   std::ifstream file(mftFile, std::ios::binary);
   if (!file.is_open()) {
       log("Error: Cannot open MFT file: " + mftFile, 0);
       return false;
   }
   
   try {
       while (!interruptFlag.load()) {
           std::vector<uint8_t> rawRecord = readRecord(file);
           if (rawRecord.empty()) {
               break;
           }
           
           try {
               log("Processing record " + std::to_string(stats.totalRecords.load()), 2);
               
               auto record = std::make_unique<MftRecord>(rawRecord, computeHashes, debug);
               
               stats.totalRecords++;
               
               if (record->flags & FILE_RECORD_IN_USE) {
                   stats.activeRecords++;
               }
               if (record->flags & FILE_RECORD_IS_DIRECTORY) {
                   stats.directories++;
               } else {
                   stats.files++;
               }
               
               uint32_t recordNum = record->recordnum;
               mftRecords[recordNum] = std::move(record);
               
               if (debug >= 2) {
                   log("Processed record " + std::to_string(stats.totalRecords.load()) + 
                       ": " + mftRecords[recordNum]->filename, 2);
               } else if (stats.totalRecords.load() % 10000 == 0) {
                   log("Processed " + std::to_string(stats.totalRecords.load()) + " records...", 1);
               }
               
               if (stats.totalRecords.load() % 1000 == 0) {
                   if (!writeCsvBlock()) {
                       log("Failed to write CSV block", 1);
                       return false;
                   }
                   mftRecords.clear();
               }
               
               if (interruptFlag.load()) {
                   log("Interrupt detected. Stopping processing.", 1);
                   break;
               }
               
           } catch (const std::exception& e) {
               log("Error processing record " + std::to_string(stats.totalRecords.load()) + 
                   ": " + e.what(), 1);
               continue;
           }
       }
       
   } catch (const std::exception& e) {
       log("Error reading MFT file: " + std::string(e.what()), 0);
       return false;
   }
   
   file.close();
   log("MFT processing complete. Total records processed: " + 
       std::to_string(stats.totalRecords.load()), 0);
   
   return true;
}

std::vector<uint8_t> MftAnalyzer::readRecord(std::ifstream& file) {
   std::vector<uint8_t> record(MFT_RECORD_SIZE);
   file.read(reinterpret_cast<char*>(record.data()), MFT_RECORD_SIZE);
   
   if (file.gcount() != MFT_RECORD_SIZE) {
       return {};
   }
   
   return record;
}

bool MftAnalyzer::initializeCsvWriter() {
   if (exportFormat == "csv") {
       csvFile = std::make_unique<std::ofstream>(outputFile);
       if (!csvFile->is_open()) {
           return false;
       }
       
       for (size_t i = 0; i < CSV_HEADER.size(); ++i) {
           *csvFile << CSV_HEADER[i];
           if (i < CSV_HEADER.size() - 1) {
               *csvFile << ",";
           }
       }
       *csvFile << "\n";
       csvFile->flush();
   }
   return true;
}

bool MftAnalyzer::writeCsvBlock() {
   if (exportFormat != "csv" || !csvFile) {
       return true;
   }
   
   log("Writing CSV block. Records in block: " + std::to_string(mftRecords.size()), 2);
   
   try {
       for (const auto& pair : mftRecords) {
           const auto& record = pair.second;
           std::string filepath = buildFilepath(record.get());
           
           std::vector<std::string> csvRow = record->toCsv();
           csvRow[8] = filepath;  // Set filepath
           
           for (size_t i = 0; i < csvRow.size(); ++i) {
               *csvFile << "\"" << csvRow[i] << "\"";
               if (i < csvRow.size() - 1) {
                   *csvFile << ",";
               }
           }
           *csvFile << "\n";
           
           if (debug >= 2) {
               log("Wrote record " + std::to_string(record->recordnum) + " to CSV", 2);
           }
       }
       
       csvFile->flush();
       log("CSV block written", 2);
       return true;
       
   } catch (const std::exception& e) {
       log("Error in writeCsvBlock: " + std::string(e.what()), 0);
       return false;
   }
}

bool MftAnalyzer::writeRemainingRecords() {
   return writeCsvBlock();
}

std::string MftAnalyzer::buildFilepath(const MftRecord* record) const {
   std::vector<std::string> pathParts;
   const MftRecord* currentRecord = record;
   int maxDepth = 255;
   
   while (currentRecord && maxDepth > 0) {
       if (currentRecord->recordnum == 5) {
           pathParts.insert(pathParts.begin(), "");
           break;
       } else if (!currentRecord->filename.empty()) {
           pathParts.insert(pathParts.begin(), currentRecord->filename);
       } else {
           pathParts.insert(pathParts.begin(), "Unknown_" + std::to_string(currentRecord->recordnum));
       }
       
       uint64_t parentRecordNum = currentRecord->getParentRecordNum();
       
       if (parentRecordNum == currentRecord->recordnum) {
           pathParts.insert(pathParts.begin(), "OrphanedFiles");
           break;
       }
       
       auto it = mftRecords.find(static_cast<uint32_t>(parentRecordNum));
       if (it != mftRecords.end()) {
           currentRecord = it->second.get();
       } else {
           pathParts.insert(pathParts.begin(), "UnknownParent_" + std::to_string(parentRecordNum));
           break;
       }
       
       maxDepth--;
   }
   
   if (maxDepth == 0) {
       pathParts.insert(pathParts.begin(), "DeepPath");
   }
   
   std::string result;
   for (size_t i = 0; i < pathParts.size(); ++i) {
       result += pathParts[i];
       if (i < pathParts.size() - 1) {
           result += "\\";
       }
   }
   
   return result;
}

bool MftAnalyzer::writeOutput() {
   log("Writing output in " + exportFormat + " format to " + outputFile, 0);
   
   try {
       if (exportFormat == "csv") {
           return writeRemainingRecords();
       } else if (exportFormat == "json") {
           JsonWriter writer;
           std::vector<const MftRecord*> records;
           for (const auto& pair : mftRecords) {
               records.push_back(pair.second.get());
           }
           return writer.write(records, outputFile);
       } else if (exportFormat == "xml") {
           XmlWriter writer;
           std::vector<const MftRecord*> records;
           for (const auto& pair : mftRecords) {
               records.push_back(pair.second.get());
           }
           return writer.write(records, outputFile);
       } else if (exportFormat == "excel") {
           ExcelWriter writer;
           std::vector<const MftRecord*> records;
           for (const auto& pair : mftRecords) {
               records.push_back(pair.second.get());
           }
           return writer.write(records, outputFile);
       } else if (exportFormat == "sqlite") {
           SqliteWriter writer;
           std::vector<const MftRecord*> records;
           for (const auto& pair : mftRecords) {
               records.push_back(pair.second.get());
           }
           return writer.write(records, outputFile);
       } else if (exportFormat == "body") {
           BodyWriter writer;
           std::vector<const MftRecord*> records;
           for (const auto& pair : mftRecords) {
               records.push_back(pair.second.get());
           }
           return writer.write(records, outputFile);
       } else if (exportFormat == "timeline") {
           TimelineWriter writer;
           std::vector<const MftRecord*> records;
           for (const auto& pair : mftRecords) {
               records.push_back(pair.second.get());
           }
           return writer.write(records, outputFile);
       } else {
           log("Unsupported export format: " + exportFormat, 0);
           return false;
       }
   } catch (const std::exception& e) {
       log("Error writing output: " + std::string(e.what()), 0);
       return false;
   }
}

void MftAnalyzer::cleanup() {
   log("Performing cleanup...", 1);
   
   if (!writeRemainingRecords()) {
       log("Failed to write remaining records during cleanup", 1);
   }
   
   if (csvFile && csvFile->is_open()) {
       csvFile->close();
   }
   
   log("Cleanup complete.", 1);
}

void MftAnalyzer::printStatistics() const {
   std::cout << "\nMFT Analysis Statistics:" << std::endl;
   std::cout << "Total records processed: " << stats.totalRecords.load() << std::endl;
   std::cout << "Active records: " << stats.activeRecords.load() << std::endl;
   std::cout << "Directories: " << stats.directories.load() << std::endl;
   std::cout << "Files: " << stats.files.load() << std::endl;
   
   if (computeHashes) {
       std::cout << "Unique MD5 hashes: " << stats.uniqueMd5.size() << std::endl;
       std::cout << "Unique SHA256 hashes: " << stats.uniqueSha256.size() << std::endl;
       std::cout << "Unique SHA512 hashes: " << stats.uniqueSha512.size() << std::endl;
       std::cout << "Unique CRC32 hashes: " << stats.uniqueCrc32.size() << std::endl;
   }
}