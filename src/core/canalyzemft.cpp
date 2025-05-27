#include "../../include/canalyemft.h"
#include "../src/core/mftAnalyzer.h"
#include <memory>
#include <string>

extern "C" {

int analyzeMft(const AnalyzeOptions* options) {
    if (!options) {
        return -1;
    }
    
    try {
        std::unique_ptr<MftAnalyzer> analyzer = std::make_unique<MftAnalyzer>(
            options->inputFile ? options->inputFile : "",
            options->outputFile ? options->outputFile : "",
            options->debug,
            options->verbosity,
            options->computeHashes != 0,
            options->exportFormat ? options->exportFormat : "csv"
        );
        
        if (analyzer->analyze()) {
            analyzer->printStatistics();
            return 0;
        } else {
            return -1;
        }
    } catch (const std::exception& e) {
        return -1;
    }
}

const char* getVersion() {
    return ANALYZEMFT_VERSION_STRING;
}

const char* getSupportedFormats() {
    return "csv,json,xml,excel,sqlite,body,timeline,tsk";
}

}