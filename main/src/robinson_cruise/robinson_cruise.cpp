#include "robinson_cruise/robinson_cruise.h"

#include <stdexcept>
#include "robinson_cruise/ternary_search.h"
#include "rust_lib.h"

double solve_robinson_cruise(Shuttle &s) {
    auto find_solution_once = [&s](std::uint64_t propose_for_aliances) {
        CStarKey *key = star_key_empty_new();
        if (key == nullptr) {
            throw std::runtime_error("Failed to allocate star key");
        }
        star_key_empty_with_fuel(key, propose_for_aliances);
        double mass_lift = star_key_fuelled_engage(key);
        star_key_destroy(key);
        std::uint64_t rest_fuel = s.calculate_rest_fuel(mass_lift, propose_for_aliances);
        std::uint64_t time_for_speed_up = s.calculate_time_for_speed_up(rest_fuel);
        std::uint64_t time_for_slow_down = s.calculate_time_for_slow_down(rest_fuel);
        return s.calculate_distance(time_for_speed_up, time_for_slow_down);
    };
    auto [_, max_distance] = ternary_search_max_int(std::uint64_t(0), s.get_total_fuel(), find_solution_once);
    return max_distance;
}
