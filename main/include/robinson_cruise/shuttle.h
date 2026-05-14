#ifndef SHUTTLE_H
#define SHUTTLE_H

#include <cstdint>

class Shuttle {
public:
    Shuttle(double mass_shuttle, double mass_fuel_unit, double power_per_unit, std::uint64_t oxygen_time, std::uint64_t total_fuel, double fuel_consumption);
    double get_mass_shuttle() const;
    double get_mass_fuel_unit() const;
    double get_power_per_unit() const;
    std::uint64_t get_oxygen_time() const;
    std::uint64_t get_total_fuel() const;
    double get_fuel_consumption() const;
    double get_distance() const;
    double get_speed() const;
    std::uint64_t calculate_rest_fuel(double mass_lift, std::uint64_t spent_fuel) const;
    std::uint64_t calculate_time_for_speed_up(std::uint64_t rest_fuel);
    std::uint64_t calculate_time_for_slow_down(std::uint64_t time_for_speed_up);
    double calculate_distance(std::uint64_t speed_up_time, std::uint64_t slow_down_time);
private:
    double newton_slowdown(double time_for_slowdown);
    double newton_slowdown_deriv(double time_for_slowdown);

    double _mass_shuttle;
    double _mass_fuel_unit;
    double _power_per_unit;
    std::uint64_t _oxygen_time;
    std::uint64_t _total_fuel;
    double _fuel_consumption;
    std::uint64_t _rest_fuel = 0;
    double _distance = 0.0;
    double _speed = 0.0;
};

#endif // SHUTTLE_H
