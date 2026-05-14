#include "star_visibility/converter.h"

#include <string>
#include <unordered_set>
#include <vector>

#include "star_visibility/moon.h"
#include "star_visibility/point.h"
#include "star_visibility/star_visibility.h"
#include "star_visibility/time_intervals.h"
#include "util.h"

using nlohmann::json;

struct MyTime {
    std::time_t t;
};

struct MyDate {
    std::time_t t;
};

namespace nlohmann {

template <>
struct adl_serializer<MyTime> {
    static void to_json(json &j, const MyTime &tp) {
        std::tm tm = *std::gmtime(&tp.t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        j = oss.str();
    }

    static void from_json(const json &j, MyTime &tp) {
        std::string s = j.get<std::string>();
        std::tm tm{};
        std::istringstream ss(s);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        if (ss.fail()) {
            throw std::runtime_error("Failed to parse time string: " + s);
        }
        tp.t = timegm(&tm);
    }
};

template <>
struct adl_serializer<MyDate> {
    static void to_json(json &j, const MyDate &tp) {
        std::tm tm = *std::gmtime(&tp.t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d");
        j = oss.str();
    }

    static void from_json(const json &j, MyDate &tp) {
        std::string s = j.get<std::string>();
        std::tm tm{};
        std::istringstream ss(s);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        if (ss.fail()) {
            throw std::runtime_error("Failed to parse time string: " + s);
        }
        tp.t = timegm(&tm);
    }
};

} // namespace nlohmann

namespace sv_converter {

struct Point {
    double x;
    double y;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Point, x, y);

struct Moon {
    std::string name;
    double orbit_radius;
    double moon_radius;
    double initial_observation_angle;
    MyTime initial_observation_time;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Moon, name, orbit_radius, moon_radius,
                                   initial_observation_angle,
                                   initial_observation_time);

struct Request {
    Point star_vector;
    std::vector<Moon> moons;
    MyDate observation_date;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Request, star_vector, moons, observation_date);

struct TimeInterval {
    MyTime start;
    MyTime end;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TimeInterval, end, start);

struct Response {
    std::vector<TimeInterval> visible_intervals;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Response, visible_intervals);
}  // namespace sv_converter

Point to_point(const sv_converter::Point &p) { return Point(p.x, p.y); }
Moon to_moon(const sv_converter::Moon &m) {
    return Moon(m.name, m.orbit_radius, m.moon_radius,
                m.initial_observation_angle,
                std::chrono::duration<double>(m.initial_observation_time.t));
}

void point_validator(const json& j) {
    std::unordered_set<std::string> allowed_keys = {"x", "y"};
    if (!j.is_object()) {
        throw std::invalid_argument("JSON value is not an object");
    }
    for (auto& [key, value] : j.items()) {
        if (allowed_keys.find(key) == allowed_keys.end()) {
            throw std::runtime_error("Unexpected key: '" + key + "'");
        }
    }
}

void moon_validator(const json& j) {
    std::unordered_set<std::string> allowed_keys = {"name", "orbit_radius", "moon_radius", "initial_observation_angle", "initial_observation_time"};
    if (!j.is_object()) {
        throw std::invalid_argument("JSON value is not an object");
    }
    for (auto& [key, value] : j.items()) {
        if (allowed_keys.find(key) == allowed_keys.end()) {
            throw std::runtime_error("Unexpected key: '" + key + "'");
        }
    }
}

template<void (*f) (const json&)>
void array_validator(const json& j) {
    if (!j.is_array()) {
        throw std::invalid_argument("JSON value is not an array");
    }
    for (const auto& item : j) {
        f(item);
    }
}

struct KV{
    char *key;
    void (*f) (const json&);
};

template<KV... keys>
void object_validator(const json& j) {
    if (!j.is_object()) {
        throw std::invalid_argument("JSON value is not an object");
    }
    for (auto& [key, value] : j.items()) {
        bool found = false;
        ([&]{
            if (keys.key == key) {
                found = true;
                keys.f(j[key]);
            }
        }(), ...);
        if (!found) {
            throw std::runtime_error("Unexpected key: '" + key + "'");
        }
    }
}

void request_validator(const json& j) {
    std::unordered_map<std::string, void (*) (const json&)> allowed_keys = {{"star_vector", point_validator}, {"moons", array_validator<moon_validator>}, {"observation_date", [](const auto &) {}}};
    if (!j.is_object()) {
        throw std::invalid_argument("JSON value is not an object");
    }
    for (auto& [key, value] : j.items()) {
        if (allowed_keys.find(key) == allowed_keys.end()) {
            throw std::runtime_error("Unexpected key: '" + key + "'");
        }
        allowed_keys[key](value);
    }
}

json process_star_visibility(const json &input) {
    request_validator(input);
    auto response = input.template get<sv_converter::Request>();
    Point start = to_point(response.star_vector);
    std::vector<Moon> moons = [&response]() {
        std::vector<Moon> result;
        result.reserve(response.moons.size());
        std::transform(response.moons.begin(), response.moons.end(),
                       std::back_inserter(result), to_moon);
        return result;
    }();
    std::time_t observation_date = response.observation_date.t;
    auto ts =
        solve_star_visibility(start, moons, observation_date).get_intervals();
    std::vector<sv_converter::TimeInterval> result_intervals;
    result_intervals.reserve(ts.size());
    std::transform(ts.begin(), ts.end(), std::back_inserter(result_intervals),
                   [](const TimeInterval &t) {
                       return sv_converter::TimeInterval{{t.get_start()},
                                                      {t.get_end()}};
                   });
    return static_cast<json>(sv_converter::Response{result_intervals});
}
