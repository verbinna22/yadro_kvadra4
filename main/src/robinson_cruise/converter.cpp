#include "robinson_cruise/converter.h"
#include "robinson_cruise/robinson_cruise.h"
#include "robinson_cruise/shuttle.h"
#include "util.h"

using nlohmann::json;

namespace rc_converter {

struct Shuttle {
    double mass_shuttle;
    double mass_fuel_unit;
    double power_per_unit;
    std::uint64_t oxygen_time;
    std::uint64_t total_fuel;
    double fuel_consumption;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Shuttle, mass_shuttle, mass_fuel_unit, power_per_unit, oxygen_time, total_fuel, fuel_consumption);

struct Response {
    double max_distance;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Response, max_distance);
} // namespace rc_converter

Shuttle to_shuttle(const rc_converter::Shuttle &s) {
    return Shuttle(s.mass_shuttle, s.mass_fuel_unit, s.power_per_unit, s.oxygen_time, s.total_fuel, s.fuel_consumption);
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

template <size_t N>
struct StringLiteral {
    // cppcheck-suppress noExplicitConstructor
    constexpr StringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
    char value[N];
};

template <StringLiteral s, void (*func) (const json&)>
struct KV {
    static constexpr auto key = s;
    static constexpr void (*f) (const json&) = func;
};

template<typename... keys>
void object_validator(const json& j) {
    if (!j.is_object()) {
        throw std::invalid_argument("JSON value is not an object");
    }
    for (auto& [key, value] : j.items()) {
        bool found = false;
        ([&]{
            if (keys::key.value == key) {
                found = true;
                keys::f(j[key]);
            }
        }(), ...);
        if (!found) {
            throw std::runtime_error("Unexpected key: '" + key + "'");
        }
    }
}

void uint64_validator(const json& j) {
    if (!j.is_number_unsigned()) {
        throw std::invalid_argument("JSON value is not an unsigned integer");
    }
}

nlohmann::json process_robinson_cruise(const nlohmann::json &input) {
    constexpr auto validate_shuttle = object_validator<KV<"mass_shuttle", [](const auto &) {}>, KV<"mass_fuel_unit", [](const auto &) {}>, KV<"power_per_unit", [](const auto &) {}>, KV<"oxygen_time", uint64_validator>, KV<"total_fuel", uint64_validator>, KV<"fuel_consumption", [](const auto &) {}>>;
    validate_shuttle(input);
    auto s = to_shuttle(input.template get<rc_converter::Shuttle>());
    double distance = solve_robinson_cruise(s);
    return static_cast<json>(rc_converter::Response{double_round(distance)});
}
