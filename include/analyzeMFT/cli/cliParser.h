#ifndef ANALYZEMFT_CLIPARSER_H
#define ANALYZEMFT_CLIPARSER_H

#include <string>
#include <vector>
#include <unordered_map>

struct CliOptions {
    std::string inputFile;
    std::string outputFile;
    std::string exportFormat = "csv";
    int verbosity = 0;
    int debug = 0;
    bool computeHashes = false;
    bool showHelp = false;
    bool showVersion = false;
};

class CliParser {
public:
    CliParser();
    
    CliOptions parse(int argc, char* argv[]);
    void printHelp() const;
    void printVersion() const;
    void printUsage() const;

private:
    std::unordered_map<std::string, std::string> longOptions;
    std::unordered_map<char, std::string> shortOptions;
    std::vector<std::string> supportedFormats;
    
    void initializeOptions();
    bool isValidFormat(const std::string& format) const;
    std::string getOptionValue(const std::string& arg, const std::string& option) const;
    void validateOptions(const CliOptions& options) const;
};

#endif