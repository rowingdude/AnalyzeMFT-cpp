#ifndef canalyzemft_H
#define canalyzemft_H

#include "version.h"
#include "../src/core/constants.h"
#include "../src/core/mftRecord.h"
#include "../src/core/mftAnalyzer.h"
#include "../src/core/winTime.h"
#include "../src/utils/hashCalc.h"
#include "../src/utils/stringUtils.h"
#include "../src/utils/logger.h"
#include "../src/utils/memUtils.h"
#include "../src/utils/fsUtils.h"
#include "../src/writers/fileWriter.h"
#include "../src/writers/csvWriter.h"
#include "../src/writers/jsonWriter.h"
#include "../src/writers/xmlWriter.h"
#include "../src/writers/excelWriter.h"
#include "../src/writers/sqliteWriter.h"
#include "../src/writers/bodyWriter.h"
#include "../src/writers/timelineWriter.h"
#include "../src/cli/cliParser.h"
#include "../src/cli/app.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* inputFile;
    const char* outputFile;
    const char* exportFormat;
    int verbosity;
    int debug;
    int computeHashes;
} AnalyzeOptions;

int analyzeMft(const AnalyzeOptions* options);
const char* getVersion();
const char* getSupportedFormats();

#ifdef __cplusplus
}
#endif

#endif