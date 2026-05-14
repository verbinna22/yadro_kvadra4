#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cmath>

#include "star_visibility/moon.h"

using namespace std::chrono_literals;

class test_moon_angle : public ::testing::TestWithParam<std::pair<double, double>> {
};

TEST_P(test_moon_angle, get_angle_returns_correct_angle) {
    auto [orbit_radius, moon_radius] = GetParam();
    Moon m("moon1", orbit_radius, moon_radius, 0.0, 0.0s);
    double angle = m.get_angle() / 180. * std::numbers::pi;
    EXPECT_DOUBLE_EQ(std::tan(angle / 2) * orbit_radius, moon_radius);
}

INSTANTIATE_TEST_SUITE_P(
    test_moon,
    test_moon_angle,
    ::testing::Values(
        std::make_pair(1.0, 0.1),
        std::make_pair(1e7, 1e5),
        std::make_pair(239e6, 361e4),
        std::make_pair(1.0, 0.999)
    )
);