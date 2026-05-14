#include "star_visibility/moon.h"

#include <cassert>
#include <cmath>
#include <numbers>

Moon::Moon(const std::string& name, double orbit_radius, double moon_radius, double initial_observation_angle, std::chrono::duration<double> initial_observation_time)
    : _name(name),
      _orbit_radius(orbit_radius),
      _moon_radius(moon_radius),
      _initial_observation_angle(initial_observation_angle),
      _speed(NAN),
      _initial_observation_time(initial_observation_time)
{
    assert(_moon_radius > 0);
    assert(_orbit_radius > 0);
    assert(_moon_radius < orbit_radius);
}

std::string Moon::get_name() const {
    return _name;
}

double Moon::get_orbit_radius() const {
    return _orbit_radius;
}

double Moon::get_moon_radius() const {
    return _moon_radius;
}

double Moon::get_initial_observation_angle() const {
    return _initial_observation_angle;
}

std::chrono::duration<double> Moon::get_initial_observation_time() const {
    return _initial_observation_time;
}

double Moon::get_speed() const {
    return _speed;
}

void Moon::set_speed(double speed) {
    _speed = speed;
}

double Moon::get_angle() const {
    return std::atan2(_moon_radius, _orbit_radius) * 360.0 / std::numbers::pi;
}
