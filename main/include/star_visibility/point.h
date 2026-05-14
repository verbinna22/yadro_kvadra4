#ifndef POINT_H
#define POINT_H

#include <cassert>
#include <cmath>
#include <numbers>
#include <stdexcept>

class Point {
public:
    Point(double x, double y) : _x(x), _y(y) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
        if (x * x + y * y == 0.0) {
#pragma GCC diagnostic pop
            throw std::invalid_argument("Zero point");
        }
    }
    double get_x() const { return _x; }
    double get_y() const { return _y; }
    double get_angle() const {
        return std::atan2(_y, _x) * 180. / std::numbers::pi;
    }
private:
    double _x;
    double _y;
};

#endif // POINT_H
