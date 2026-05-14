#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "star_visibility/point.h"

class test_point_angle : public ::testing::TestWithParam<std::pair<double, double>> {
};

TEST_P(test_point_angle, get_angle_returns_correct_angle) {
    auto [x, y] = GetParam();
    Point p(x, y);
    double angle = p.get_angle();
    EXPECT_DOUBLE_EQ(std::tan(angle * std::numbers::pi / 180.), y / x);
}

TEST(test_point, zero_point_throws_exception) {
    EXPECT_THROW(Point(0.0, 0.0), std::invalid_argument);
}

INSTANTIATE_TEST_SUITE_P(
    test_point,
    test_point_angle,
    ::testing::Values(
        std::make_pair(1., 1.),
        std::make_pair(1e7, 1e5),
        std::make_pair(239e6, 361e4),
        std::make_pair(1.0, 0.999)
    )
);


