#include "logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

std::unique_ptr<Logger> Logger::instance = nullptr;
std::mutex Logger::instanceMutex;

Logger::Logger(int verbosityLevel, const std::string& logFile) 
    : verbosityLevel(verbosityLevel) {
    if (!logFile.empty()) {
        this->logFile = std::make_unique<std::ofstream>(logFile, std::ios::app);
    }
}

Logger::~Logger() {
    if (logFile && logFile->is_open()) {
        logFile->close();
    }
}

Logger& Logger::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (!instance) {
        instance = std::make_unique<Logger>();
    }
    return *instance;
}

void Logger::log(const std::string& message, LogLevel level) {
    writeLog(message, level);
}

void Logger::debug(const std::string& message) {
    log(message, LogLevel::DEBUG);
}

void Logger::info(const std::string& message) {
    log(message, LogLevel::INFO);
}

void Logger::warning(const std::string& message) {
    log(message, LogLevel::WARNING);
}

void Logger::error(const std::string& message) {
    log(message, LogLevel::ERROR);
}

void Logger::setVerbosityLevel(int level) {
    verbosityLevel = level;
}

int Logger::getVerbosityLevel() const {
    return verbosityLevel;
}

void Logger::writeLog(const std::string& message, LogLevel level) {
    if (static_cast<int>(level) < verbosityLevel) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = levelToString(level);
    std::string logMessage = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    std::cout << logMessage << std::endl;
    
    if (logFile && logFile->is_open()) {
        *logFile << logMessage << std::endl;
        logFile->flush();
    }
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}