#ifndef STAR_VISIBILITY_H
#define STAR_VISIBILITY_H

#include <chrono>

#include "point.h"
#include "moon.h"
#include "time_intervals.h"

TimeIntervals solve_star_visibility(const Point &start, std::span<Moon> moons, const std::time_t &observation_date);

#endif // STAR_VISIBILITY_H
