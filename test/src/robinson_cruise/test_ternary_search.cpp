#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cmath>

#include "robinson_cruise/ternary_search.h"

class test_ternary_search : public ::testing::TestWithParam<std::tuple<std::int64_t, double, double, std::int64_t, std::int64_t>> {
};

TEST_P(test_ternary_search, correct_values) {
    auto [xx, a, yy, l, r] = GetParam();
    ASSERT_EQ(ternary_search_max_int(l, r,
        [&](std::int64_t x) { return -a * std::pow(static_cast<double>(x - xx), 2) + yy; }
    ).first, xx);
}

INSTANTIATE_TEST_SUITE_P(
    test_ternary_search,
    test_ternary_search,
    ::testing::Values(
        std::make_tuple(0, 1., 0., -10, 10),
        std::make_tuple(5, 1., 0., -10, 10),
        std::make_tuple(5, 1., 1.5, -10, 10),
        std::make_tuple(5, 2., 1.5, -10, 10),
        std::make_tuple(5, 1., 0., -1000000000, 1000000000)
    )
);
