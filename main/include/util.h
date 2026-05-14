#ifndef UTIL_H
#define UTIL_H

#include <cmath>

inline double double_round(double val) {
    return std::round(val * 10.0) / 10.0;
}

#endif // UTIL_H
