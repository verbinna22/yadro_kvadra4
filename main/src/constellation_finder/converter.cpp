#include "constellation_finder/converter.h"
#include "constellation_finder/constellation.h"
#include "constellation_finder/constellation_finder.h"
#include "constellation_finder/edge.h"

using nlohmann::json;

namespace cf_converter {

struct Edge {
    int from;
    int to;
    double distance;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Edge, from, to, distance);

struct Constellation {
    std::vector<Edge> edges;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Constellation, edges);

struct Request {
    std::vector<std::string> star_hashes;
    Constellation target_constellation;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Request, star_hashes, target_constellation);

struct Response {
    bool found;
    std::optional<std::vector<std::string>> result;
};

template <typename BasicJsonType,
          nlohmann ::detail ::enable_if_t<
              nlohmann ::detail ::is_basic_json<BasicJsonType>::value, int> = 0>
void to_json(BasicJsonType &nlohmann_json_j, const Response &nlohmann_json_t) {
    nlohmann_json_j["found"] = nlohmann_json_t.found;
    if (nlohmann_json_t.found) {
        nlohmann_json_j["result"] = nlohmann_json_t.result;
    }
}
template <typename BasicJsonType,
          nlohmann ::detail ::enable_if_t<
              nlohmann ::detail ::is_basic_json<BasicJsonType>::value, int> = 0>
void from_json(const BasicJsonType &nlohmann_json_j,
               Response &nlohmann_json_t) {
    nlohmann_json_j.at("found").get_to(nlohmann_json_t.found);
    nlohmann_json_j.at("result").get_to(nlohmann_json_t.result);
};

} // namespace cf_converter

Edge to_edge(const cf_converter::Edge &e) {
    return Edge(e.from, e.to, e.distance);
}

Constellation to_constellation(const cf_converter::Constellation &c) {
    std::vector<Edge> edges;
    edges.reserve(c.edges.size());
    std::transform(c.edges.begin(), c.edges.end(), std::back_inserter(edges), to_edge);
    return Constellation(edges);
}

cf_converter::Response to_response(const std::vector<std::string> &result) {
    if (result.empty()) {
        return cf_converter::Response{false, std::nullopt};
    } else {
        return cf_converter::Response{true, result};
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

void int_validator(const json& j) {
    if (!j.is_number_integer()) {
        throw std::invalid_argument("JSON value is not an integer");
    }
    auto value = j.get<nlohmann::json::number_integer_t>();
    if (value > std::numeric_limits<int>::max() || value < std::numeric_limits<int>::min()) {
        throw std::runtime_error("Integer value out of range");
    }
}

nlohmann::json process_constellation_finder(const nlohmann::json &input) {
    constexpr auto validate_edge = object_validator<KV<"from", int_validator>, KV<"to", int_validator>, KV<"distance", [](const auto &) {}>>;
    constexpr auto validate_edges = array_validator<validate_edge>;
    constexpr auto validate_constellation = object_validator<KV<"edges", validate_edges>>;
    constexpr auto validate_request = object_validator<KV<"star_hashes", array_validator<[](const auto &) {}>>, KV<"target_constellation", validate_constellation>>;
    validate_request(input);
    auto response = input.template get<cf_converter::Request>();
    std::vector<std::string> star_hashes = response.star_hashes;
    Constellation c = to_constellation(response.target_constellation);
    auto result = to_response(solve_constellation_finder(star_hashes, c));
    return static_cast<json>(result);
}
