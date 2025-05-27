#ifndef ANALYZEMFT_MFTANALYZER_H
#define ANALYZEMFT_MFTANALYZER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <fstream>
#include <atomic>
#include <future>
#include <vector>
#include "mftRecord.h"

struct AnalysisStats {
    std::atomic<uint64_t> totalRecords{0};
    std::atomic<uint64_t> activeRecords{0};
    std::atomic<uint64_t> directories{0};
    std::atomic<uint64_t> files{0};
    std::unordered_set<std::string> uniqueMd5;
    std::unordered_set<std::string> uniqueSha256;
    std::unordered_set<std::string> uniqueSha512;
    std::unordered_set<std::string> uniqueCrc32;
};

class MftAnalyzer {
public:
    MftAnalyzer(const std::string& mftFile, const std::string& outputFile, 
                int debug = 0, int verbosity = 0, bool computeHashes = false, 
                const std::string& exportFormat = "csv");
    
    ~MftAnalyzer();
    
    bool analyze();
    void cleanup();
    void printStatistics() const;
    
    void setInterruptFlag() { interruptFlag = true; }
    bool isInterrupted() const { return interruptFlag; }

private:
    std::string mftFile;
    std::string outputFile;
    int debug;
    int verbosity;
    bool computeHashes;
    std::string exportFormat;
    
    std::atomic<bool> interruptFlag{false};
    std::unordered_map<uint32_t, std::unique_ptr<MftRecord>> mftRecords;
    AnalysisStats stats;
    
    std::unique_ptr<std::ofstream> csvFile;
    std::unique_ptr<std::ostream> csvWriter;
    
    bool processMft();
    std::vector<uint8_t> readRecord(std::ifstream& file);
    bool initializeCsvWriter();
    bool writeCsvBlock();
    bool writeRemainingRecords();
    bool writeOutput();
    std::string buildFilepath(const MftRecord* record) const;
    
    void log(const std::string& message, int level = 0) const;
    void setupInterruptHandler();
    
    static void signalHandler(int signal);
    static MftAnalyzer* currentInstance;
};

#endif