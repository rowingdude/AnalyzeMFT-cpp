#include "../../include/analyzeMFT/cli/app.h"
#include "../utils/logger.h"
#include "../utils/fsUtils.h"
#include "../../include/version.h"
#include <iostream>
#include <csignal>

Application* Application::currentInstance = nullptr;

Application::Application() : parser(std::make_unique<CliParser>()) {
    currentInstance = this;
    setupSignalHandlers();
}

Application::~Application() {
    currentInstance = nullptr;
}

int Application::run(int argc, char* argv[]) {
    try {
        printBanner();
        
        CliOptions options = parser->parse(argc, argv);
        
        if (options.showHelp) {
            parser->printHelp();
            return 0;
        }
        
        if (options.showVersion) {
            parser->printVersion();
            return 0;
        }
        
        if (!validateInputs(options)) {
            return 1;
        }
        
        if (!initializeAnalyzer(options)) {
            return 1;
        }
        
        Logger::getInstance().setVerbosityLevel(std::max(options.verbosity, options.debug));
        
        if (!analyzer->analyze()) {
            std::cerr << "Analysis failed" << std::endl;
            return 1;
        }
        
        analyzer->printStatistics();
        std::cout << "Analysis complete. Results written to " << options.outputFile << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        parser->printUsage();
        return 1;
    }
}

bool Application::validateInputs(const CliOptions& options) {
    if (!FileSystemUtils::fileExists(options.inputFile)) {
        std::cerr << "Error: Input file '" << options.inputFile << "' does not exist." << std::endl;
        return false;
    }
    
    if (!FileSystemUtils::isReadable(options.inputFile)) {
        std::cerr << "Error: Cannot read input file '" << options.inputFile << "'." << std::endl;
        return false;
    }
    
    std::string outputDir = FileSystemUtils::getDirname(options.outputFile);
    if (!FileSystemUtils::fileExists(outputDir)) {
        if (!FileSystemUtils::createDirectories(outputDir)) {
            std::cerr << "Error: Cannot create output directory '" << outputDir << "'." << std::endl;
            return false;
        }
    }
    
    if (FileSystemUtils::fileExists(options.outputFile) && !FileSystemUtils::isWritable(options.outputFile)) {
        std::cerr << "Error: Cannot write to output file '" << options.outputFile << "'." << std::endl;
        return false;
    }
    
    return true;
}

bool Application::initializeAnalyzer(const CliOptions& options) {
    try {
        analyzer = std::make_unique<MftAnalyzer>(
            options.inputFile,
            options.outputFile,
            options.debug,
            options.verbosity,
            options.computeHashes,
            options.exportFormat
        );
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing analyzer: " << e.what() << std::endl;
        return false;
    }
}

void Application::setupSignalHandlers() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
}

void Application::signalHandler(int signal) {
    if (currentInstance && currentInstance->analyzer) {
        std::cout << "\nReceived interrupt signal. Cleaning up..." << std::endl;
        currentInstance->analyzer->setInterruptFlag();
    }
}

void Application::printBanner() const {
    std::cout << "AnalyzeMFT C++ - NTFS Master File Table Analyzer" << std::endl;
    std::cout << "Version " << ANALYZEMFT_VERSION_STRING << std::endl;
    std::cout << "=========================================" << std::endl << std::endl;
}