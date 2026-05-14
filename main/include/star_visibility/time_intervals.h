#ifndef TIME_INTERVALS_H
#define TIME_INTERVALS_H

#include <vector>

#include "time_interval.h"

class TimeIntervals {
public:
    std::vector<TimeInterval> get_intervals() const;
    void cut_interval(double start, double end);

    static TimeIntervals get_default(const std::time_t &start_day);
    static constexpr std::size_t SECONDS_IN_DAY = 24 * 60 * 60;
private:
    explicit TimeIntervals(const std::time_t &start_day);

    std::int64_t get_index(double border);

    static constexpr std::size_t ARRAY_SIZE = 2 * SECONDS_IN_DAY - 1;
    std::time_t _start_day;
    std::vector<bool> _nofree_seconds;
};

#endif // TIME_INTERVALS_H
