#ifndef TIME_INTERVAL_H
#define TIME_INTERVAL_H

#include <chrono>

class TimeInterval {
public:
    TimeInterval(std::time_t start, std::time_t end): _start(start), _end(end) {}
    std::time_t get_start() const { return _start; }
    std::time_t get_end() const { return _end; }
private:
    std::time_t _start;
    std::time_t _end;
};

#endif // TIME_INTERVAL_H
