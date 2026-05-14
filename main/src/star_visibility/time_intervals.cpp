#include "star_visibility/time_intervals.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

TimeIntervals::TimeIntervals(const std::time_t &start_day)
    : _start_day(start_day), _nofree_seconds(ARRAY_SIZE) {}

std::vector<TimeInterval> TimeIntervals::get_intervals() const {
    std::vector<TimeInterval> result;
    auto begin = _nofree_seconds.begin();
    auto iter = begin;
    auto end = _nofree_seconds.end();
    while ((iter = std::find(iter, end, 0)) != end) {
        auto next = std::find(iter, end, 1);
        result.emplace_back(TimeInterval(_start_day + (iter - begin + 1) / 2, _start_day + (next - begin - 1) / 2));
        iter = next;
    }
    return result;
}

void TimeIntervals::cut_interval(double start, double end) {
    assert(start < end);
    using diff_t = std::vector<bool>::difference_type;
    auto low_set_one = static_cast<diff_t>(std::clamp<std::int64_t>(get_index(start), 0, ARRAY_SIZE));
    auto high_set_one = static_cast<diff_t>(std::clamp<std::int64_t>(get_index(end) + 1, 0, ARRAY_SIZE));
    std::fill(_nofree_seconds.begin() + low_set_one, _nofree_seconds.begin() + high_set_one, true);
}

TimeIntervals TimeIntervals::get_default(const std::time_t &start_day) {
    return TimeIntervals(start_day);
}

std::int64_t TimeIntervals::get_index(double border) {
    double int_part;
    if (border < 0.0) {
        return -1;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
    bool is_accurate = std::modf(border, &int_part) == 0.0;
#pragma GCC diagnostic pop
    return 2 * (static_cast<std::int64_t>(int_part) - static_cast<std::int64_t>(_start_day)) + static_cast<std::int64_t>(!is_accurate);
}
