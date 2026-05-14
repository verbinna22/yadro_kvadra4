#ifndef TERNARY_SEARCH_H
#define TERNARY_SEARCH_H

#include <utility>

template<typename Func, typename Int>
std::pair<Int, double> ternary_search_max_int(Int left, Int right, Func f) {
    while (right - left > 2) {
        Int m1 = left + (right - left) / 3;
        Int m2 = right - (right - left) / 3;
        if (f(m1) < f(m2)) {
            left = m1;
        } else {
            right = m2;
        }
    }
    Int best = left;
    double f_best = f(best);
    for (Int i = left + 1; i <= right; ++i) {
        double f_i = f(i);
        if (f_i > f_best) {
            best = i;
            f_best = f_i;
        }
    }
    return std::make_pair(best, f_best);
}

#endif // TERNARY_SEARCH_H
