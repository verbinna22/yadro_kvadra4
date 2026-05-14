#include "robinson_cruise/shuttle.h"

#include <stdexcept>

#include <boost/math/special_functions/digamma.hpp>
#include <boost/math/special_functions/trigamma.hpp>

Shuttle::Shuttle(double mass_shuttle, double mass_fuel_unit, double power_per_unit, std::uint64_t oxygen_time, std::uint64_t total_fuel, double fuel_consumption)
    : _mass_shuttle(mass_shuttle),
      _mass_fuel_unit(mass_fuel_unit),
      _power_per_unit(power_per_unit),
      _oxygen_time(oxygen_time),
      _total_fuel(total_fuel),
      _fuel_consumption(fuel_consumption)
{
    if (mass_shuttle < 1e-10 || mass_fuel_unit < 0.0 || power_per_unit < 0.0 || fuel_consumption < 0.0) {
        throw std::invalid_argument("Negative arguments");
    }
}

double Shuttle::get_mass_shuttle() const {
    return _mass_shuttle;
}

double Shuttle::get_mass_fuel_unit() const {
    return _mass_fuel_unit;
}

double Shuttle::get_power_per_unit() const {
    return _power_per_unit;
}

std::uint64_t Shuttle::get_oxygen_time() const {
    return _oxygen_time;
}

std::uint64_t Shuttle::get_total_fuel() const {
    return _total_fuel;
}

double Shuttle::get_fuel_consumption() const {
    return _fuel_consumption;
}

double Shuttle::get_distance() const {
    return _distance;
}

double Shuttle::get_speed() const {
    return _speed;
}

std::uint64_t Shuttle::calculate_rest_fuel(double mass_lift, std::uint64_t spent_fuel) const {
    const double fuel_share_to_lift = 1 / (_mass_fuel_unit * _fuel_consumption + 1);
    std::uint64_t fuel = _total_fuel - spent_fuel;
    if (mass_lift > _mass_shuttle) {
        double fuel_up_free = (mass_lift - _mass_shuttle) / _mass_fuel_unit;
        return static_cast<std::uint64_t>(fuel_up_free + fuel_share_to_lift * (static_cast<double>(fuel) - fuel_up_free));
    }
    double rest_fuel = std::max(static_cast<double>(fuel) - (_mass_shuttle - mass_lift) * _fuel_consumption, 0.0);
    return static_cast<std::uint64_t>(rest_fuel * fuel_share_to_lift);
}

std::uint64_t Shuttle::calculate_time_for_speed_up(std::uint64_t rest_fuel) {
    std::uint64_t time_for_speed_up = std::min(_oxygen_time / 2, rest_fuel / 2);
    _rest_fuel = rest_fuel - time_for_speed_up;
    if (time_for_speed_up == 0) {
        _speed = 0.0;
        _distance = 0.0;
        return time_for_speed_up;
    }
    if (_mass_fuel_unit < 1e-10) {
        _speed = _power_per_unit * static_cast<double>(time_for_speed_up) / _mass_shuttle;
        _distance = _power_per_unit * static_cast<double>(time_for_speed_up) * static_cast<double>(time_for_speed_up) / _mass_shuttle/ 2.;
        return time_for_speed_up;
    }
    double digamma_diff = boost::math::digamma(_mass_shuttle / _mass_fuel_unit + static_cast<double>(rest_fuel))
           - boost::math::digamma(_mass_shuttle / _mass_fuel_unit + static_cast<double>(rest_fuel - time_for_speed_up));
    _speed = _power_per_unit / _mass_fuel_unit * digamma_diff;
    // _distance = _power_per_unit * (static_cast<double>(time_for_speed_up) / _mass_fuel_unit - ((static_cast<double>(time_for_speed_up - rest_fuel) + 0.5) / _mass_fuel_unit - _mass_shuttle / _mass_fuel_unit / _mass_fuel_unit) * digamma_diff);
    _distance = _power_per_unit * (static_cast<double>(time_for_speed_up) - (_mass_shuttle / _mass_fuel_unit + static_cast<double>(rest_fuel - time_for_speed_up) - 0.5) * digamma_diff) / _mass_fuel_unit;
    return time_for_speed_up;
}

std::uint64_t Shuttle::calculate_time_for_slow_down(std::uint64_t time_for_speed_up) {
    std::uint64_t time_for_slowdown_int;
    if (time_for_speed_up == 0 || _speed < 1e-10) {
        return 0;
    }
    if (_mass_fuel_unit < 1e-10) {
        time_for_slowdown_int = time_for_speed_up;
        _distance *= 2;
        return time_for_slowdown_int;
    }
    
    double time_for_slowdown = 0.0;
    double new_time_for_slowdown = static_cast<double>(time_for_speed_up);
    constexpr double epsilon = 1e-10;
    while (std::abs(new_time_for_slowdown - time_for_slowdown) > epsilon) {
        time_for_slowdown = new_time_for_slowdown;
        double derivative = newton_slowdown_deriv(time_for_slowdown);
        if (std::abs(derivative) < epsilon) {
            break;
        }
        new_time_for_slowdown = time_for_slowdown - newton_slowdown(time_for_slowdown) / derivative;
        if (new_time_for_slowdown < 0.) {
            new_time_for_slowdown = time_for_slowdown / 2;
        }
    }
    std::uint64_t time_for_slowdown_int_prob = static_cast<std::uint64_t>(time_for_slowdown);
    for (time_for_slowdown_int = std::max(time_for_slowdown_int_prob, static_cast<std::uint64_t>(2)) - 2; time_for_slowdown_int <= time_for_slowdown_int_prob + 2; ++time_for_slowdown_int) {
        if (newton_slowdown(static_cast<double>(time_for_slowdown_int + 1)) < 0.0) {
            break;
        }
    }
    double digamma_diff = boost::math::digamma(_mass_shuttle / _mass_fuel_unit + static_cast<double>(_rest_fuel))
           - boost::math::digamma(_mass_shuttle / _mass_fuel_unit + static_cast<double>(_rest_fuel - time_for_slowdown_int));
    double final_speed = _speed - _power_per_unit / _mass_fuel_unit * digamma_diff;
    double final_distance = static_cast<double>(time_for_slowdown_int) * _speed - _power_per_unit * (static_cast<double>(time_for_slowdown_int) - ((static_cast<double>(_rest_fuel - time_for_slowdown_int) - 0.5) + _mass_shuttle / _mass_fuel_unit) * digamma_diff) / _mass_fuel_unit;
    _distance += final_distance;
    if (final_speed > epsilon) {
        double final_acceleration = _power_per_unit / (_mass_shuttle + static_cast<double>(_rest_fuel - time_for_slowdown_int) * _mass_fuel_unit);
        double final_time = final_speed / final_acceleration;
        _distance += (final_speed * final_time - (final_acceleration * final_time * final_time) / 2.);
        return time_for_slowdown_int + 1;
    }
    return time_for_slowdown_int;
}

double Shuttle::calculate_distance(std::uint64_t speed_up_time, std::uint64_t slow_down_time) {
    std::uint64_t time_for_move = _oxygen_time - speed_up_time - slow_down_time;
    _distance += static_cast<double>(time_for_move) * _speed;
    return _distance;
}

double Shuttle::newton_slowdown(double time_for_slowdown) {
    double digamma_diff = boost::math::digamma(_mass_shuttle / _mass_fuel_unit + static_cast<double>(_rest_fuel))
           - boost::math::digamma(_mass_shuttle / _mass_fuel_unit + static_cast<double>(_rest_fuel) - time_for_slowdown);
    return _speed - _power_per_unit / _mass_fuel_unit * digamma_diff;
}

double Shuttle::newton_slowdown_deriv(double time_for_slowdown) {
    return - _power_per_unit / _mass_fuel_unit * boost::math::trigamma(_mass_shuttle / _mass_fuel_unit + static_cast<double>(_rest_fuel) - time_for_slowdown);
}
