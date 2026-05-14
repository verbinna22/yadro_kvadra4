#ifndef MOON_H
#define MOON_H

#include <string>
#include <chrono>

class Moon {
public:
    Moon(const std::string& name, double orbit_radius, double moon_radius, double initial_observation_angle, std::chrono::duration<double> initial_observation_time);
    std::string get_name() const;
    double get_orbit_radius() const;
    double get_moon_radius() const;
    double get_initial_observation_angle() const;
    std::chrono::duration<double> get_initial_observation_time() const;
    double get_speed() const;
    void set_speed(double speed);
    double get_angle() const;
private:
    std::string _name;
    double _orbit_radius;
    double _moon_radius;
    double _initial_observation_angle;
    double _speed;
    std::chrono::duration<double> _initial_observation_time;
};

#endif // MOON_H
