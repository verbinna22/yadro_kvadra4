#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <format>
#include <stdexcept>

#include "constellation_finder/constellation.h"
#include "constellation_finder/constellation_finder.h"
#include "constellation_finder/edge.h"

TEST(solve_constellation_finder, one_to_one) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 1.)
    });
    std::vector<std::string> star_hashes {
        "#1#0.0",
        "#1#1.0",
        "#1#2.0",
        "#1#3.0"
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 4);
    ASSERT_EQ(result[0], "#1#0.0");
    ASSERT_EQ(result[1], "#1#1.0");
    ASSERT_EQ(result[2], "#1#2.0");
    ASSERT_EQ(result[3], "#1#3.0");
}

TEST(solve_constellation_finder, different_order) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 1.)
    });
    std::vector<std::string> star_hashes {
        "#1#3.0",
        "#1#1.0",
        "#1#0.0",
        "#1#2.0"
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 4);
    ASSERT_EQ(result[0], "#1#3.0");
    ASSERT_EQ(result[1], "#1#2.0");
    ASSERT_EQ(result[2], "#1#1.0");
    ASSERT_EQ(result[3], "#1#0.0");
}

TEST(solve_constellation_finder, imbalance_first) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 2.)
    });
    std::vector<std::string> star_hashes {
        "#1#3.0",
        "#1#1.0",
        "#1#-1.0",
        "#1#2.0"
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 4);
    ASSERT_EQ(result[0], "#1#3.0");
    ASSERT_EQ(result[1], "#1#2.0");
    ASSERT_EQ(result[2], "#1#1.0");
    ASSERT_EQ(result[3], "#1#-1.0");
}

TEST(solve_constellation_finder, imbalance_second) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 0.9)
    });
    std::vector<std::string> star_hashes {
        "#1#2.9",
        "#1#1.0",
        "#1#0.0",
        "#1#2.0"
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 4);
    ASSERT_EQ(result[0], "#1#0.0");
    ASSERT_EQ(result[1], "#1#1.0");
    ASSERT_EQ(result[2], "#1#2.0");
    ASSERT_EQ(result[3], "#1#2.9");
}

TEST(solve_constellation_finder, different_distances) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 1.1)
    });
    std::vector<std::string> star_hashes {
        "#1#4.0",
        "#1#1.0",
        "#1#0.0",
        "#1#2.0"
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 4);
    ASSERT_EQ(result[0], "#1#0.0");
    ASSERT_EQ(result[1], "#1#1.0");
    ASSERT_EQ(result[2], "#1#2.0");
    ASSERT_EQ(result[3], "#1#4.0");
}

TEST(solve_constellation_finder, isolated_stars) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 1.1)
    });
    std::vector<std::string> star_hashes {
        "a#4.0",
        "a#1.0",
        "a#0.0",
        "c#10.0",
        "a#2.0",
        "b#5.0"
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 4);
    ASSERT_EQ(result[0], "a#0.0");
    ASSERT_EQ(result[1], "a#1.0");
    ASSERT_EQ(result[2], "a#2.0");
    ASSERT_EQ(result[3], "a#4.0");
}

TEST(solve_constellation_finder, incorrect_constellation_not_connected) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(2, 3, 1.1)
    });
    std::vector<std::string> star_hashes {
        "a#4.0",
        "a#1.0",
        "a#0.0",
        "c#10.0",
        "a#2.0",
        "b#5.0"
    };
    ASSERT_THROW(solve_constellation_finder(star_hashes, c), std::invalid_argument);
}

TEST(solve_constellation_finder, incorrect_constellation_cycle) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 1.1),
        Edge(0, 3, 2.1)
    });
    std::vector<std::string> star_hashes {
        "a#4.0",
        "a#1.0",
        "a#0.0",
        "c#10.0",
        "a#2.0",
        "b#5.0"
    };
    ASSERT_THROW(solve_constellation_finder(star_hashes, c), std::invalid_argument);
}

TEST(solve_constellation_finder, incorrect_constellation_cycle_and_not_connected) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 1.1),
        Edge(0, 3, 2.1),
        Edge(5, 4, 1.),
    });
    std::vector<std::string> star_hashes {
        "a#4.0",
        "a#1.0",
        "a#0.0",
        "c#10.0",
        "a#2.0",
        "b#5.0"
    };
    ASSERT_THROW(solve_constellation_finder(star_hashes, c), std::invalid_argument);
}

TEST(solve_constellation_finder, incorrect_constellation_negative_distance) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, -1.),
        Edge(2, 3, 1.1)
    });
    std::vector<std::string> star_hashes {
        "a#4.0",
        "a#1.0",
        "a#0.0",
        "c#10.0",
        "a#2.0",
        "b#5.0"
    };
    ASSERT_THROW(solve_constellation_finder(star_hashes, c), std::invalid_argument);
}

TEST(solve_constellation_finder, incorrect_stars_no_stars) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 1.1)
    });
    std::vector<std::string> star_hashes {
        "a#4.0",
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 0);
}

TEST(solve_constellation_finder, different_edges_size) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.05),
        Edge(2, 3, 1.1)
    });
    std::vector<std::string> star_hashes {
        "a#10.0",
        "a#1.0",
        "a#0.0",
        "a#2.5"
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 4);
    ASSERT_EQ(result[0], "a#0.0");
    ASSERT_EQ(result[1], "a#1.0");
    ASSERT_EQ(result[2], "a#2.5");
    ASSERT_EQ(result[3], "a#10.0");
}

TEST(solve_constellation_finder, incorrect_edges_size) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.05),
        Edge(2, 3, 1.02)
    });
    std::vector<std::string> star_hashes {
        "a#10.0",
        "a#1.0",
        "a#0.0",
        "a#2.5"
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 0);
}

TEST(solve_constellation_finder, different_stars_size) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.05),
        Edge(2, 3, 1.1)
    });
    std::vector<std::string> star_hashes {
        "a#10.0",
        "a#1.0",
        "a#0.0"
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 0);
}

TEST(solve_constellation_finder, similar_distances_ok) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 2.),
        Edge(2, 4, 2.)
    });
    std::vector<std::string> star_hashes {
        "a#0.0#0.0",
        "a#0.0#2.0",
        "a#0.0#4.0",
        "a#-5.0#4.0",
        "a#5.0#4.0",
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 5);
    ASSERT_EQ(result[0], "a#0.0#0.0");
    ASSERT_EQ(result[1], "a#0.0#2.0");
    ASSERT_EQ(result[2], "a#0.0#4.0");
    ASSERT_EQ(result[3], "a#-5.0#4.0");
    ASSERT_EQ(result[4], "a#5.0#4.0");
}

TEST(solve_constellation_finder, many_groups) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 2.),
        Edge(2, 4, 2.)
    });
    std::vector<std::string> star_hashes {
        "a#0.0#0.0",
        "a#0.0#2.0",
        "a#0.0#4.0",
        "a#-5.0#4.0",
        "a#5.0#4.0",
        "b#0.0#0.0",
        "b#0.0#2.0",
        "b#0.0#4.0",
        "b#-5.0#4.0",
        "b#6.0#4.0",
        "c#0.0#-1.0",
        "c#0.0#2.0",
        "c#0.0#4.0",
        "c#-5.0#4.0",
        "c#5.0#4.0",
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 5);
    ASSERT_EQ(result[0], "a#0.0#0.0");
    ASSERT_EQ(result[1], "a#0.0#2.0");
    ASSERT_EQ(result[2], "a#0.0#4.0");
    ASSERT_EQ(result[3], "a#-5.0#4.0");
    ASSERT_EQ(result[4], "a#5.0#4.0");
}

TEST(solve_constellation_finder, many_appropriate_groups) {
    Constellation c({
        Edge(0, 1, 1.),
        Edge(1, 2, 1.),
        Edge(2, 3, 2.),
        Edge(2, 4, 2.)
    });
    std::vector<std::string> star_hashes {
        "a#0.0#0.0",
        "a#0.0#2.0",
        "a#0.0#4.0",
        "a#-5.0#4.0",
        "a#5.0#4.0",
        "b#0.0#0.0",
        "b#0.0#2.0",
        "b#0.0#4.0",
        "b#-5.0#4.0",
        "b#5.0#4.0",
        "c#0.0#0.0",
        "c#0.0#2.0",
        "c#0.0#4.0",
        "c#-5.0#4.0",
        "c#5.0#4.0",
    };
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 0);
}

TEST(solve_constellation_finder, critical_groups) {
    std::vector edges {
        Edge(3, 1, 1.),
        Edge(1, 2, 2.),
        Edge(1, 0, 2.)
    };
    std::vector<std::string> star_hashes_template {
        "a#0.0#2.0",
        "a#0.0#4.0",
        "a#-5.0#4.0",
        "a#5.0#4.0",
    };
    std::vector<std::string> star_hashes_bad_template {
        "0.0#2.0",
        "0.0#4.0",
        "-5.0#4.0",
        "6.0#4.0",
    };
    for (int i = 0; i < 50; ++i) {
        edges.push_back(Edge(3 + i, 4 + i, 1));
        star_hashes_template.push_back(std::format("a#0.0#{}.0", -2 * i));
        star_hashes_bad_template.push_back(std::format("0.0#{}.0", -2 * i));
    }
    Constellation c(edges);
    std::vector<std::string> star_hashes;
    std::copy(star_hashes_template.begin(), star_hashes_template.end(), std::back_inserter(star_hashes));
    for (char a = 'b'; a <= 'z'; ++a) {
        std::transform(star_hashes_bad_template.begin(), star_hashes_bad_template.end(), std::back_inserter(star_hashes), [a](std::string &templ) { return std::format("{}{}{}#{}", a, a, a, templ); }
        );
    }
    auto result = solve_constellation_finder(star_hashes, c);
    ASSERT_EQ(result.size(), 54);
}
