#include "cliParser.h"
#include "../include/version.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>

CliParser::CliParser() {
    initializeOptions();
}

void CliParser::initializeOptions() {
    supportedFormats = {"csv", "json", "xml", "excel", "sqlite", "body", "timeline", "tsk"};
    
    longOptions = {
        {"--file", "inputFile"},
        {"--output", "outputFile"},
        {"--csv", "csv"},
        {"--json", "json"},
        {"--xml", "xml"},
        {"--excel", "excel"},
        {"--sqlite", "sqlite"},
        {"--body", "body"},
        {"--timeline", "timeline"},
        {"--tsk", "tsk"},
        {"--hash", "computeHashes"},
        {"--help", "showHelp"},
        {"--version", "showVersion"}
    };
    
    shortOptions = {
        {'f', "inputFile"},
        {'o', "outputFile"},
        {'H', "computeHashes"},
        {'v', "verbosity"},
        {'d', "debug"},
        {'h', "showHelp"}
    };
}

CliOptions CliParser::parse(int argc, char* argv[]) {
    CliOptions options;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            options.showHelp = true;
        } else if (arg == "--version") {
            options.showVersion = true;
        } else if (arg == "--file" || arg == "-f") {
            if (i + 1 < argc) {
                options.inputFile = argv[++i];
            } else {
                throw std::runtime_error("Option " + arg + " requires a value");
            }
        } else if (arg == "--output" || arg == "-o") {
            if (i + 1 < argc) {
                options.outputFile = argv[++i];
            } else {
                throw std::runtime_error("Option " + arg + " requires a value");
            }
        } else if (arg == "--hash" || arg == "-H") {
            options.computeHashes = true;
        } else if (arg == "-v") {
            options.verbosity++;
        } else if (arg == "-d") {
            options.debug++;
        } else if (arg.substr(0, 2) == "--" && isValidFormat(arg.substr(2))) {
            options.exportFormat = arg.substr(2);
        } else if (arg.find("=") != std::string::npos) {
            size_t pos = arg.find("=");
            std::string key = arg.substr(0, pos);
            std::string value = arg.substr(pos + 1);
            
            if (key == "--file" || key == "-f") {
                options.inputFile = value;
            } else if (key == "--output" || key == "-o") {
                options.outputFile = value;
            }
        } else if (arg.substr(0, 1) == "-" && arg.length() > 1) {
            throw std::runtime_error("Unknown option: " + arg);
        } else {
            if (options.inputFile.empty()) {
                options.inputFile = arg;
            } else if (options.outputFile.empty()) {
                options.outputFile = arg;
            }
        }
    }
    
    validateOptions(options);
    return options;
}

void CliParser::validateOptions(const CliOptions& options) const {
    if (options.showHelp || options.showVersion) {
        return;
    }
    
    if (options.inputFile.empty()) {
        throw std::runtime_error("Input file is required. Use -f or --file to specify an MFT file.");
    }
    
    if (options.outputFile.empty()) {
        throw std::runtime_error("Output file is required. Use -o or --output to specify an output file.");
    }
    
    if (!isValidFormat(options.exportFormat)) {
        throw std::runtime_error("Unsupported export format: " + options.exportFormat);
    }
}

bool CliParser::isValidFormat(const std::string& format) const {
    return std::find(supportedFormats.begin(), supportedFormats.end(), format) != supportedFormats.end();
}

void CliParser::printHelp() const {
    std::cout << "Usage: analyzemft [OPTIONS] -f <input_file> -o <output_file>\n\n";
    std::cout << "Analyze NTFS Master File Table (MFT) files\n\n";
    std::cout << "Required Arguments:\n";
    std::cout << "  -f, --file FILE          MFT file to analyze\n";
    std::cout << "  -o, --output FILE        Output file\n\n";
    std::cout << "Export Format Options:\n";
    std::cout << "  --csv                    Export as CSV (default)\n";
    std::cout << "  --json                   Export as JSON\n";
    std::cout << "  --xml                    Export as XML\n";
    std::cout << "  --excel                  Export as Excel\n";
    std::cout << "  --sqlite                 Export as SQLite database\n";
    std::cout << "  --body                   Export as body file (for mactime)\n";
    std::cout << "  --timeline               Export as TSK timeline\n";
    std::cout << "  --tsk                    Export as TSK bodyfile format\n\n";
    std::cout << "Other Options:\n";
    std::cout << "  -H, --hash               Compute hashes (MD5, SHA256, SHA512, CRC32)\n";
    std::cout << "  -v                       Increase output verbosity (can be used multiple times)\n";
    std::cout << "  -d                       Increase debug output (can be used multiple times)\n";
    std::cout << "  -h, --help               Show this help message\n";
    std::cout << "  --version                Show version information\n\n";
    std::cout << "Examples:\n";
    std::cout << "  analyzemft -f mft.raw -o output.csv\n";
    std::cout << "  analyzemft -f mft.raw -o output.json --json -H -v\n";
    std::cout << "  analyzemft --file mft.raw --output analysis.sqlite --sqlite --hash\n";
}

void CliParser::printVersion() const {
    std::cout << "AnalyzeMFT C++ version " << ANALYZEMFT_VERSION_STRING << std::endl;
    std::cout << "Built from Python analyzeMFT by Benjamin Cance" << std::endl;
}

void CliParser::printUsage() const {
    std::cout << "Usage: analyzemft -f <mft_file> -o <output_file> [options]" << std::endl;
    std::cout << "Use --help for detailed usage information." << std::endl;
}