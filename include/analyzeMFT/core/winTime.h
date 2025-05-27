#ifndef ANALYZEMFT_WINTIME_H
#define ANALYZEMFT_WINTIME_H

#include <cstdint>
#include <string>
#include <chrono>

class WindowsTime {
public:
    WindowsTime(uint32_t low, uint32_t high);
    WindowsTime();
    
    std::string getDateTimeString() const;
    std::time_t getUnixTime() const;
    bool isValid() const;
    
    uint32_t low;
    uint32_t high;
    std::time_t unixTime;
    std::string dtstr;
    bool valid;

private:
    void calculateUnixTime();
    void formatDateTime();
};

#endif