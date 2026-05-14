#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <stdexcept>

#include "robinson_cruise/shuttle.h"

class test_shuttle_constructor_throws : public ::testing::TestWithParam<std::tuple<double, double, double, std::uint64_t, std::uint64_t, double>> {
};

TEST_P(test_shuttle_constructor_throws, incorrect_values_throws_exception) {
    auto [ms, mfu, ppu, ot, tf, fc] = GetParam();
    ASSERT_THROW(Shuttle(ms, mfu, ppu, ot, tf, fc), std::invalid_argument);
}

INSTANTIATE_TEST_SUITE_P(
    test_shuttle_constructor_throws,
    test_shuttle_constructor_throws,
    ::testing::Values(
        std::make_tuple(-1., 1., 1., 0, 0, 1.),
        std::make_tuple(1e-20, 1., 1., 0, 0, 1.),
        std::make_tuple(1., -1., 1., 0, 0, 1.),
        std::make_tuple(1., 1., -1., 0, 0, 1.),
        std::make_tuple(1., 1., 1., 0, 0, -1.),
        std::make_tuple(-1., 1., 1., 1, 1, 1.)
    )
);

class test_shuttle_constructor_no_throws : public ::testing::TestWithParam<std::tuple<double, double, double, std::uint64_t, std::uint64_t, double>> {
};

TEST_P(test_shuttle_constructor_no_throws, correct_values_not_throws_exception) {
    auto [ms, mfu, ppu, ot, tf, fc] = GetParam();
    ASSERT_NO_THROW(Shuttle(ms, mfu, ppu, ot, tf, fc));
}

INSTANTIATE_TEST_SUITE_P(
    test_shuttle_constructor_no_throws,
    test_shuttle_constructor_no_throws,
    ::testing::Values(
        std::make_tuple(1., 0., 0., 0, 0, 0.),
        std::make_tuple(1., 0., 1., 1, 1, 1.),
        std::make_tuple(1., 1., 0., 1, 1, 1.),
        std::make_tuple(1., 1., 1., 0, 1, 1.),
        std::make_tuple(1., 1., 1., 1, 0, 1.),
        std::make_tuple(1., 1., 1., 1, 1, 0.),
        std::make_tuple(1., 1., 1., 1, 1, 1.)
    )
);

class test_calculate_rest_fuel : public ::testing::TestWithParam<std::tuple<Shuttle, double, std::uint64_t, std::uint64_t>> {
};

TEST_P(test_calculate_rest_fuel, correct_values) {
    auto [shuttle, mass_lift, spent_fuel, rest_fuel] = GetParam();
    ASSERT_EQ(shuttle.calculate_rest_fuel(mass_lift, spent_fuel), rest_fuel);
}

INSTANTIATE_TEST_SUITE_P(
    test_calculate_rest_fuel,
    test_calculate_rest_fuel,
    ::testing::Values(
        std::make_tuple(Shuttle(1000., 1., 1., 1, 4000, 3.), 0., 0, 250),
        std::make_tuple(Shuttle(1000., 3., 1., 1, 4000, 3.), 0., 0, 100),
        std::make_tuple(Shuttle(1200., 3., 1., 1, 4000, 3.), 200., 0, 100),
        std::make_tuple(Shuttle(1200., 3., 1., 1, 4100, 3.), 200., 100, 100),
        std::make_tuple(Shuttle(1000., 3., 1., 1, 4100, 3.), 1000., 100, 400),
        std::make_tuple(Shuttle(1000., 3., 1., 1, 4200, 3.), 1300., 100, 500)
    )
);

class test_calculate_time_for_speed_up : public ::testing::TestWithParam<std::tuple<Shuttle, std::uint64_t>> {
};

static std::uint64_t calculate_time_for_speed_up(Shuttle &s, std::uint64_t rest_fuel, double &distance, double &speed) {
    std::uint64_t time_for_speed_up = std::min(s.get_oxygen_time() / 2, rest_fuel / 2);
    speed = 0.0;
    distance = 0.0;
    for (std::uint64_t i = 0; i < time_for_speed_up; ++i) {
        double acceleration = s.get_power_per_unit() / (s.get_mass_shuttle() + s.get_mass_fuel_unit() * static_cast<double>(rest_fuel - 1 - i));
        distance += speed + 0.5 * acceleration;
        speed += acceleration;
    }
    return time_for_speed_up;
}

TEST_P(test_calculate_time_for_speed_up, correct_values) {
    auto [shuttle, rest_fuel] = GetParam();
    double distance = 0.0;
    double speed = 0.0;
    std::uint64_t time_for_speed_up = calculate_time_for_speed_up(shuttle, rest_fuel, distance, speed);
    std::uint64_t result = shuttle.calculate_time_for_speed_up(rest_fuel);
    EXPECT_EQ(result, time_for_speed_up);
    EXPECT_NEAR(distance, shuttle.get_distance(), 1e-7);
    EXPECT_NEAR(speed, shuttle.get_speed(), 1e-7);
}

INSTANTIATE_TEST_SUITE_P(
    test_calculate_time_for_speed_up,
    test_calculate_time_for_speed_up,
    ::testing::Values(
        std::make_tuple(Shuttle(1000., 1., 1., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 3., 1., 100, 4000, 3.), 100),
        std::make_tuple(Shuttle(1., 1., 1., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 100, 4000, 3.), 0),
        std::make_tuple(Shuttle(1000., 0., 1., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 0., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 0, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 100, 0, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 100, 4000, 0.), 250),
        std::make_tuple(Shuttle(1., 0., 0., 100, 0, 0.), 0),
        std::make_tuple(Shuttle(1., 0., 0., 0, 0, 0.), 0)
    )
);

class test_calculate_time_for_slow_down : public ::testing::TestWithParam<std::tuple<Shuttle, std::uint64_t>> {
};

static std::uint64_t calculate_time_for_slow_down(Shuttle &s, std::uint64_t rest_fuel, double &distance, double speed) {
    std::uint64_t result = 0;
    while (speed > 1e-10) {
        double acceleration = s.get_power_per_unit() / (s.get_mass_shuttle() + s.get_mass_fuel_unit() * static_cast<double>(rest_fuel - 1 - result));
        result++;
        double new_speed = speed - acceleration;
        if (new_speed < 1e-10) {
            double time = speed / acceleration;
            distance += speed * time - 0.5 * acceleration * time * time;
            break;
        }
        distance += speed - 0.5 * acceleration;
        speed = new_speed;
    }
    return result;
}

TEST_P(test_calculate_time_for_slow_down, correct_values) {
    auto [shuttle, rest_fuel] = GetParam();
    double distance = 0.0;
    double speed = 0.0;
    std::uint64_t time_for_speed_up = calculate_time_for_speed_up(shuttle, rest_fuel, distance, speed);
    std::uint64_t time_for_slow_down = calculate_time_for_slow_down(shuttle, rest_fuel - time_for_speed_up, distance, speed);
    EXPECT_EQ(shuttle.calculate_time_for_speed_up(rest_fuel), time_for_speed_up);
    std::uint64_t result = shuttle.calculate_time_for_slow_down(time_for_speed_up);
    EXPECT_EQ(result, time_for_slow_down);
    EXPECT_NEAR(distance, shuttle.get_distance(), 1e-6);
    EXPECT_NEAR(speed, shuttle.get_speed(), 1e-6);
}

INSTANTIATE_TEST_SUITE_P(
    test_calculate_time_for_slow_down,
    test_calculate_time_for_slow_down,
    ::testing::Values(
        std::make_tuple(Shuttle(1000., 1., 1., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 3., 1., 100, 4000, 3.), 100),
        std::make_tuple(Shuttle(1., 1., 1., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 100, 4000, 3.), 0),
        std::make_tuple(Shuttle(1000., 0., 1., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 0., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 0, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 100, 0, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 100, 4000, 0.), 250),
        std::make_tuple(Shuttle(1., 0., 0., 100, 0, 0.), 0),
        std::make_tuple(Shuttle(1., 0., 0., 0, 0, 0.), 0)
    )
);

class test_calculate_distance : public ::testing::TestWithParam<std::tuple<Shuttle, std::uint64_t>> {
};

static double calculate_distance(Shuttle &s, std::uint64_t speed_up_time, std::uint64_t slow_down_time, double distance, double speed) {
    return static_cast<double>(s.get_oxygen_time() - speed_up_time - slow_down_time) * speed + distance;
}

TEST_P(test_calculate_distance, correct_values) {
    auto [shuttle, rest_fuel] = GetParam();
    double distance = 0.0;
    double speed = 0.0;
    std::uint64_t time_for_speed_up = calculate_time_for_speed_up(shuttle, rest_fuel, distance, speed);
    std::uint64_t time_for_slow_down = calculate_time_for_slow_down(shuttle, rest_fuel - time_for_speed_up, distance, speed);
    EXPECT_EQ(shuttle.calculate_time_for_speed_up(rest_fuel), time_for_speed_up);
    EXPECT_EQ(shuttle.calculate_time_for_slow_down(time_for_speed_up), time_for_slow_down);
    distance = calculate_distance(shuttle, time_for_speed_up, time_for_slow_down, distance, speed);
    EXPECT_NEAR(distance, shuttle.calculate_distance(time_for_speed_up, time_for_slow_down), 1e-6);
}

INSTANTIATE_TEST_SUITE_P(
    test_calculate_distance,
    test_calculate_distance,
    ::testing::Values(
        std::make_tuple(Shuttle(1000., 1., 1., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 3., 1., 100, 4000, 3.), 100),
        std::make_tuple(Shuttle(1., 1., 1., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 100, 4000, 3.), 0),
        std::make_tuple(Shuttle(1000., 0., 1., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 0., 100, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 0, 4000, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 100, 0, 3.), 250),
        std::make_tuple(Shuttle(1000., 1., 1., 100, 4000, 0.), 250),
        std::make_tuple(Shuttle(1., 0., 0., 100, 0, 0.), 0),
        std::make_tuple(Shuttle(1., 0., 0., 0, 0, 0.), 0)
    )
);
