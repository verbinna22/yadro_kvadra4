#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "star_visibility/moon.h"
#include "star_visibility/point.h"
#include "star_visibility/star_visibility.h"
#include "star_visibility/time_intervals.h"

using namespace std::chrono_literals;

TEST(test_solve_star_visibility, slow_moons_are_free) {
    std::vector<Moon> moons {
        Moon("#172800#1.145877", 1e8, 1e6, 0., 0.0s),
        Moon("#345600#1.145877", 1e9, 1e7, 0., 0.0s),
    };
    std::vector<TimeInterval> ts = solve_star_visibility(Point(0., 1.), moons, 0).get_intervals();
    ASSERT_EQ(ts.size(), 1);
    ASSERT_EQ(ts[0].get_start(), 0);
    ASSERT_EQ(ts[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_solve_star_visibility, fast_moon_are_closed) {
    std::vector<Moon> moons {
        Moon("#1#1.145877", 1e8, 1e6, 90., 0.0s),
        Moon("#345600#1.145877", 1e9, 1e7, 0., 0.0s),
    };
    std::vector<TimeInterval> ts = solve_star_visibility(Point(0., 1.), moons, 0).get_intervals();
    ASSERT_EQ(ts.size(), 0);
}

TEST(test_solve_star_visibility, fast_moon2_are_free) {
    std::vector<Moon> moons {
        Moon("#1#1.145877", 1e8, 1e6, 0., 0.0s),
        Moon("#345600#1.145877", 1e9, 1e7, 0., 0.0s),
    };
    std::vector<TimeInterval> ts = solve_star_visibility(Point(0., 1.), moons, 0).get_intervals();
    ASSERT_EQ(ts.size(), TimeIntervals::SECONDS_IN_DAY);
    for (std::size_t i = 0; i < ts.size(); ++i) {
        ASSERT_EQ(ts[i].get_start(), i);
        ASSERT_EQ(ts[i].get_end(), i);
    }
}

TEST(test_solve_star_visibility, speeds_are_correct) {
    std::vector<Moon> moons {
        Moon("#1#1.145877", 1e8, 1e6, 0., 0.0s),
        Moon("#345600#1.145877", 1e9, 1e7, 0., 0.0s),
    };
    solve_star_visibility(Point(0., 1.), moons, 0);
    ASSERT_NEAR(moons[0].get_speed(), 360. / 1.0, 1e-6);
    ASSERT_NEAR(moons[1].get_speed(), 360. / 345600.0, 1e-6);
}

TEST(test_solve_star_visibility, incorrect_moon_throws_exception) {
    std::vector<Moon> moons {
        Moon("Moon1", 1e8, 1e6, 0., 0.0s),
        Moon("#345600#1.145877", 1e9, 1e7, 0., 0.0s),
    };
    ASSERT_THROW(solve_star_visibility(Point(0., 1.), moons, 0), std::invalid_argument);
}

TEST(test_solve_star_visibility, non_standard_vector_free) {
    std::vector<Moon> moons {
        Moon("#172800#1.145877", 1e8, 1e6, 0., 0.0s),
        Moon("#345600#1.145877", 1e9, 1e7, 0., 0.0s),
    };
    std::vector<TimeInterval> ts = solve_star_visibility(Point(-1., 2.), moons, 0).get_intervals();
    ASSERT_EQ(ts.size(), 1);
    ASSERT_EQ(ts[0].get_start(), 0);
    ASSERT_EQ(ts[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_solve_star_visibility, lots_of_moons_free) {
    std::vector<Moon> moons {
        Moon("#172800#1.145877", 1e8, 1e6, 0., 0.0s),
        Moon("#345600#1.145877", 1e9, 1e7, 0., 0.0s),
        Moon("#500000#1.145877", 1e7, 1e5, 0., 0.0s),
    };
    std::vector<TimeInterval> ts = solve_star_visibility(Point(0., 1.), moons, 0).get_intervals();
    ASSERT_EQ(ts.size(), 1);
    ASSERT_EQ(ts[0].get_start(), 0);
    ASSERT_EQ(ts[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_solve_star_visibility, one_moon_cuts_intervals) {
    std::vector<Moon> moons {
        Moon("#54000#1.145877", 1e8, 1e6, 0., 0.0s),
        Moon("#345600#1.145877", 1e9, 1e7, 0., 0.0s),
    };
    std::vector<TimeInterval> ts = solve_star_visibility(Point(0., 1.), moons, 0).get_intervals();
    ASSERT_EQ(ts.size(), 2);
}

TEST(test_solve_star_visibility, moons_cut_intervals) {
    std::vector<Moon> moons {
        Moon("#54000#1.145877", 1e8, 1e6, 0., 0.0s),
        Moon("#64800#1.145877", 1e9, 1e7, 0., 0.0s),
    };
    std::vector<TimeInterval> ts = solve_star_visibility(Point(0., 1.), moons, 0).get_intervals();
    ASSERT_EQ(ts.size(), 3);
}

TEST(test_solve_star_visibility, one_moon_cuts_intervals_correct) {
    std::vector<Moon> moons {
        Moon("#54000#1.145877", 1e8, 1e6, 0.57293869, 0.0s),
        Moon("#345600#1.145877", 1e9, 1e7, 0., 0.0s),
    };
    std::vector<TimeInterval> ts = solve_star_visibility(Point(0., 1.), moons, 0).get_intervals();
    ASSERT_EQ(ts.size(), 2);
    ASSERT_EQ(ts[0].get_start(), 0);
    ASSERT_EQ(ts[0].get_end(), 40500);
    ASSERT_EQ(ts[1].get_start(), 40672);
    ASSERT_EQ(ts[1].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_solve_star_visibility, one_moon_cuts_intervals_another_day) {
    std::vector<Moon> moons {
        Moon("#54000#1.145877", 1e8, 1e6, 0., 1234.0s),
        Moon("#800000#1.145877", 1e9, 1e7, 0., 0.0s),
    };
    std::vector<TimeInterval> ts = solve_star_visibility(Point(0., 1.), moons, 86400).get_intervals();
    ASSERT_EQ(ts.size(), 3);
}
