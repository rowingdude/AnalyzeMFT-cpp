#include "winTime.h"
#include <ctime>
#include <iomanip>
#include <sstream>

WindowsTime::WindowsTime(uint32_t low, uint32_t high) 
    : low(low), high(high), unixTime(0), valid(false) {
    if (low == 0 && high == 0) {
        dtstr = "Not defined";
        return;
    }
    calculateUnixTime();
    formatDateTime();
}

WindowsTime::WindowsTime() : low(0), high(0), unixTime(0), dtstr("Not defined"), valid(false) {
}

void WindowsTime::calculateUnixTime() {
    uint64_t t = (static_cast<uint64_t>(high) << 32) | low;
    unixTime = static_cast<std::time_t>((t / 10000000ULL) - 11644473600ULL);
    valid = true;
}

void WindowsTime::formatDateTime() {
    if (!valid) {
        dtstr = "Invalid timestamp";
        return;
    }
    
    std::tm* utcTime = std::gmtime(&unixTime);
    if (!utcTime) {
        dtstr = "Invalid timestamp";
        valid = false;
        return;
    }
    
    std::ostringstream oss;
    oss << std::put_time(utcTime, "%Y-%m-%dT%H:%M:%S") << "Z";
    dtstr = oss.str();
}

std::string WindowsTime::getDateTimeString() const {
    return dtstr;
}

std::time_t WindowsTime::getUnixTime() const {
    return unixTime;
}

bool WindowsTime::isValid() const {
    return valid;
}