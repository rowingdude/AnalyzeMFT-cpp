#ifndef ANALYZEMFT_APP_H
#define ANALYZEMFT_APP_H

#include "cliParser.h"
#include "../core/mftAnalyzer.h"
#include <memory>

class Application {
public:
    Application();
    ~Application();
    
    int run(int argc, char* argv[]);

private:
    std::unique_ptr<CliParser> parser;
    std::unique_ptr<MftAnalyzer> analyzer;
    
    bool validateInputs(const CliOptions& options);
    bool initializeAnalyzer(const CliOptions& options);
    void setupSignalHandlers();
    void printBanner() const;
    
    static void signalHandler(int signal);
    static Application* currentInstance;
};

#endif