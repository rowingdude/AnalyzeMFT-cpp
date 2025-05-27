#ifndef ANALYZEMFT_LOGGER_H
#define ANALYZEMFT_LOGGER_H

#include <string>
#include <fstream>
#include <memory>
#include <mutex>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

class Logger {
public:
    Logger(int verbosityLevel = 1, const std::string& logFile = "");
    ~Logger();
    
    void log(const std::string& message, LogLevel level = LogLevel::INFO);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    void setVerbosityLevel(int level);
    int getVerbosityLevel() const;
    
    static Logger& getInstance();

private:
    int verbosityLevel;
    std::unique_ptr<std::ofstream> logFile;
    std::mutex logMutex;
    
    std::string getCurrentTimestamp() const;
    std::string levelToString(LogLevel level) const;
    void writeLog(const std::string& message, LogLevel level);
    
    static std::unique_ptr<Logger> instance;
    static std::mutex instanceMutex;
};

#endif