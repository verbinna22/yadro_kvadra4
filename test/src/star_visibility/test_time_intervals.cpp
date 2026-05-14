#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <random>
#include <utility>
#include <vector>

#include "star_visibility/time_intervals.h"

TEST(test_time_intervals, get_default_returns_big_interval) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 1);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, get_default_shift_returns_big_interval_shift) {
    std::time_t shift = 12345;
    TimeIntervals ti = TimeIntervals::get_default(shift);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 1);
    ASSERT_EQ(intervals[0].get_start(), shift);
    ASSERT_EQ(intervals[0].get_end(), shift + static_cast<std::time_t>(TimeIntervals::SECONDS_IN_DAY) - 1);
}

TEST(test_time_intervals, cut_interval_splits_big_interval) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(1.0, 2.0);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 2);
}

TEST(test_time_intervals, cut_interval_splits_integer_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(1.0, 2.0);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 2);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), 0);
    ASSERT_EQ(intervals[1].get_start(), 3);
    ASSERT_EQ(intervals[1].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_small_splits_integer_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(1.1, 1.9);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 2);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), 1);
    ASSERT_EQ(intervals[1].get_start(), 2);
    ASSERT_EQ(intervals[1].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_small_splits_after_begin_integer_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(0.1, 0.9);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 2);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), 0);
    ASSERT_EQ(intervals[1].get_start(), 1);
    ASSERT_EQ(intervals[1].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_small_splits_before_begin_integer_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(-0.9, -0.1);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 1);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_small_splits_before_end_integer_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(TimeIntervals::SECONDS_IN_DAY - 1.5, TimeIntervals::SECONDS_IN_DAY - 1.1);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 2);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 2);
    ASSERT_EQ(intervals[1].get_start(), TimeIntervals::SECONDS_IN_DAY - 1);
    ASSERT_EQ(intervals[1].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_small_splits_after_end_integer_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(TimeIntervals::SECONDS_IN_DAY - 0.9, TimeIntervals::SECONDS_IN_DAY - 0.1);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 1);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_splits_float_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(1.5, 2.5);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 2);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), 1);
    ASSERT_EQ(intervals[1].get_start(), 3);
    ASSERT_EQ(intervals[1].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_splits_several_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(1.5, 2.5);
    ti.cut_interval(5.0, 10.5);
    ti.cut_interval(20.0, 50.5);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 4);
}

TEST(test_time_intervals, cut_interval_splits_close_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(1.5, 2.5);
    ti.cut_interval(2.7, 10.5);
    ti.cut_interval(10.6, 50.5);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 2);
}

TEST(test_time_intervals, cut_interval_splits_intersects_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(1.5, 2.5);
    ti.cut_interval(1.7, 10.5);
    ti.cut_interval(1.1, 50.5);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 2);
}

TEST(test_time_intervals, cut_interval_splits_first_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(-1.5, 2.5);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 1);
    ASSERT_EQ(intervals[0].get_start(), 3);
    ASSERT_EQ(intervals[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_splits_2negs_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(-1.5, -0.5);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 1);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_splits_2big_negs_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(-10.5, -5.5);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 1);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_splits_2poss_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(static_cast<double>(TimeIntervals::SECONDS_IN_DAY) - 0.5, static_cast<double>(TimeIntervals::SECONDS_IN_DAY));
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 1);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_splits_2big_poss_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(1e10, 2e10);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 1);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), TimeIntervals::SECONDS_IN_DAY - 1);
}

TEST(test_time_intervals, cut_interval_all_empty_result) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    for (std::size_t i = 0; i < TimeIntervals::SECONDS_IN_DAY; ++i) {
        double border = static_cast<double>(i);
        ti.cut_interval(border - 0.1, border + 0.1);
    }
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 0);
}

TEST(test_time_intervals, cut_interval_all_accurate_end_empty_result) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    for (std::size_t i = 0; i < TimeIntervals::SECONDS_IN_DAY; ++i) {
        double border = static_cast<double>(i);
        ti.cut_interval(border - 0.1, border);
    }
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 0);
}

TEST(test_time_intervals, cut_interval_all_accurate_begin_empty_result) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    for (std::size_t i = 0; i < TimeIntervals::SECONDS_IN_DAY; ++i) {
        double border = static_cast<double>(i);
        ti.cut_interval(border, border + 0.1);
    }
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 0);
}

TEST(test_time_intervals, cut_interval_splits_last_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(2.5, 1e10);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 1);
    ASSERT_EQ(intervals[0].get_start(), 0);
    ASSERT_EQ(intervals[0].get_end(), 2);
}

TEST(test_time_intervals, cut_interval_empty_correct) {
    TimeIntervals ti = TimeIntervals::get_default(0);
    ti.cut_interval(-1e10, 1e10);
    auto intervals = ti.get_intervals();
    ASSERT_EQ(intervals.size(), 0);
}

TEST(test_time_intervals, cut_interval_random) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> cut_number_dist(0, 1000);
    std::uniform_real_distribution<> value_dist(-static_cast<double>(TimeIntervals::SECONDS_IN_DAY) - 1, static_cast<double>(TimeIntervals::SECONDS_IN_DAY) + 1);
    int iterations = 1000;
    for (int i = 0; i < iterations; ++i) {
        TimeIntervals ti = TimeIntervals::get_default(0);
        int cut_number = cut_number_dist(gen);
        std::vector<std::pair<double, double>> cuts;
        for (int j = 0; j < cut_number; ++j) {
            double start = value_dist(gen);
            double end = value_dist(gen);
            if (start > end) {
                std::swap(start, end);
            }
            ti.cut_interval(start, end);
            cuts.emplace_back(start, end);
        }
        auto intervals = ti.get_intervals();
        for (size_t k = 0; k < intervals.size(); ++k) {
            ASSERT_LE(intervals[k].get_start(), intervals[k].get_end());
            if (k > 0) {
                ASSERT_GT(intervals[k].get_start(), intervals[k - 1].get_end());
            }
            ASSERT_GE(intervals[k].get_start(), 0);
            ASSERT_LE(intervals[k].get_end(), static_cast<std::time_t>(TimeIntervals::SECONDS_IN_DAY) - 1);
            bool is_extremum_start = (intervals[k].get_start() == 0);
            bool is_extremum_end = (intervals[k].get_end() == static_cast<std::time_t>(TimeIntervals::SECONDS_IN_DAY) - 1);
            for (const auto &[start, end] : cuts) {
                ASSERT_TRUE(static_cast<double>(intervals[k].get_end()) < start || static_cast<double>(intervals[k].get_start()) > end);
                if (static_cast<double>(intervals[k].get_end()) < start && static_cast<double>(intervals[k].get_end() + 1) >= start) {
                    is_extremum_end = true;
                }
                if (static_cast<double>(intervals[k].get_start()) > end && static_cast<double>(intervals[k].get_start() - 1) <= end) {
                    is_extremum_start = true;
                }
            }
            ASSERT_TRUE(is_extremum_start);
            ASSERT_TRUE(is_extremum_end);
        }
    }
}

