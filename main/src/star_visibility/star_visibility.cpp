#include <cassert>
#include <cmath>
#include <stdexcept>

#include "rust_lib.h"
#include "star_visibility/star_visibility.h"
#include "star_visibility/time_intervals.h"

static void calculate_moons_speed(Moon &moon_1, Moon &moon_2);
static double angular_distance(double a, double b);

TimeIntervals solve_star_visibility(const Point &start,
                                    std::span<Moon> moons,
                                    const std::time_t &observation_date) {
    assert(moons.size() >= 2);
    double start_ray = start.get_angle();
    TimeIntervals result = TimeIntervals::get_default(observation_date);
    for (std::size_t i = 0; i + 1 < moons.size(); i += 2) {
        calculate_moons_speed(moons[i], moons[i + 1]);
    }
    if (moons.size() % 2) {
        calculate_moons_speed(moons[0], moons.back());
    }
    for (const auto &moon : moons) {
        double date_seconds = static_cast<double>(observation_date) - moon.get_initial_observation_time().count();
        double moon_angle = std::fmod((moon.get_initial_observation_angle() - moon.get_speed() * static_cast<double>(date_seconds)), 360.);
        if (moon_angle < 0.) {
            moon_angle += 360.;
        }
        double angle_left = moon_angle + moon.get_angle() / 2;
        double angle_right = moon_angle - moon.get_angle() / 2;
        double start_intersect;
        double end_intersect;
        double ri_dist;
        double le_dist;
        if ((le_dist = angular_distance(angle_left, start_ray)) <= 0) {
            ri_dist = angular_distance(angle_right, start_ray);
            start_intersect = static_cast<double>(date_seconds) - ri_dist / moon.get_speed();
            end_intersect = static_cast<double>(date_seconds) - le_dist / moon.get_speed();
        } else {
            if ((ri_dist = angular_distance(angle_right, start_ray)) > 0) {
                start_intersect = static_cast<double>(date_seconds) + (360. - ri_dist) / moon.get_speed();
            } else {
                start_intersect = static_cast<double>(date_seconds) - ri_dist / moon.get_speed();
            }
            end_intersect = static_cast<double>(date_seconds) + (360. - le_dist) / moon.get_speed();
        }
        double period = 360. / moon.get_speed();
        for (std::uint64_t i = 0; start_intersect + period * static_cast<double>(i) < static_cast<double>(date_seconds + TimeIntervals::SECONDS_IN_DAY); ++i) {
            result.cut_interval(start_intersect + period * static_cast<double>(i), end_intersect + period * static_cast<double>(i));
        }
    }
    return result;
}

static void calculate_moons_speed(Moon &moon_1, Moon &moon_2) {
    double angle_1 = moon_1.get_angle();
    double angle_2 = moon_2.get_angle();
    CLunarData data;
    switch (light_analyzer_engage(moon_1.get_name().c_str(),
                               moon_2.get_name().c_str(), &data)) {
        case CLightErrorCode_Ok:
            break;
        case CLightErrorCode_InvalidLunarIdentifier:
            throw std::invalid_argument("Invalid lunar identifier");
        case CLightErrorCode_ModuleOverheat:
        case CLightErrorCode_InvalidStateError:
            throw std::logic_error("Failed to run lib");
    }
    double time_diff = static_cast<double>(data.eclipse_time_diff_secs) +
                       static_cast<double>(data.eclipse_time_diff_nanos) / 1e9;
    double speed_diff = (360. - angle_1 - angle_2) / time_diff;
    double mean_period =
        static_cast<double>(data.orbital_period_avg_secs) +
        static_cast<double>(data.orbital_period_avg_nanos) / 1e9;
    double sum_to_prod = mean_period / 180.;
    double discriminant = std::pow(sum_to_prod * speed_diff, 2) + 4;
    double speed_1 =
        (2.0 - sum_to_prod * speed_diff + std::sqrt(discriminant)) /
        (2.0 * sum_to_prod);
    double speed_2 = speed_1 + speed_diff;
    if (data.faster_moon == CFasterMoon_FirstOne) {
        moon_1.set_speed(speed_2);
        moon_2.set_speed(speed_1);
    } else {
        moon_1.set_speed(speed_1);
        moon_2.set_speed(speed_2);
    }
}

static double angular_distance(double a, double b) {
    double diff = b - a;
    if (diff > 180.) {
        diff -= 360.;
    } else if (diff <= -180.) {
        diff += 360.;
    }
    return diff;
}
