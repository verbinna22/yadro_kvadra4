#include <algorithm>
#include <cassert>
#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <limits>

#include "constellation_finder/constellation_finder.h"
#include "constellation_finder/edge.h"
#include "rust_lib.h"

using Graph = std::unordered_map<int, std::vector<std::tuple<int, int>>>;
constexpr std::size_t first_id = std::numeric_limits<size_t>::max();

static void distribute_constellations(const std::vector<std::string> &star_hashes,
               std::vector<std::vector<Edge>> &star_groups,
               std::vector<int> &star_id_to_group_id);
static std::unordered_map<int, int> leave_tree(std::vector<Edge> &star_groups);
static std::unordered_map<int, int> unite_tree(const std::vector<Edge> &star_groups);
static int get_root(int star_id, std::unordered_map<int, int> &star_to_root);
static int get_or_default(std::unordered_map<int, int> &map, int star_id, int default_value = -1);
static void unite(int root1, int root2, std::unordered_map<int, int> &star_to_root, std::unordered_map<int, int> &root_to_size);
static std::vector<std::string> are_iso(Graph &example, Graph &to_find, int example_start, int to_find_start, const std::vector<std::string> &star_hashes);
static void find_centroids(Graph &g, std::vector<int> &centroids);
static Graph transform_to_graph(const std::vector<Edge> &edges);
static std::vector<std::string> are_iso(const std::vector<Edge> &c, const std::vector<Edge> &edges, const std::vector<std::string> &star_hashes);
static int get_hash(Graph &g, int start, std::map<std::vector<std::pair<int, int>>, int> &hashes, std::unordered_map<int, std::vector<std::tuple<int, int, int>>> &sorted_children);
static int get_hash(const std::vector<std::tuple<int, int, int>> &cs, std::map<std::vector<std::pair<int, int>>, int> &hashes);
static void check_constellation(const std::vector<Edge> &edges);

std::vector<std::string> solve_constellation_finder(
    const std::vector<std::string> &star_hashes, Constellation &c) {
    const std::vector<Edge> &edges = c.get_edges();
    check_constellation(edges);
    if (star_hashes.size() > 1000) {
        throw std::invalid_argument("star hashes");
    }
    std::vector<std::vector<Edge>> star_groups;
    std::vector<int> star_id_to_group_id(star_hashes.size(), -1);
    distribute_constellations(star_hashes, star_groups, star_id_to_group_id);
    for (auto& group : star_groups) {
        std::sort(group.begin(), group.end(), [](const Edge& a, const Edge& b) {
            return a.get_distance() < b.get_distance();
        });
    }
    Graph edges_graph = transform_to_graph(edges);
    std::vector<std::string> id_to_hash(edges_graph.size());
    std::vector<std::string> result;
    for (auto &group : star_groups) {
        leave_tree(group);
        if (group.size() == edges.size()) {
            std::vector<std::string> local_result = are_iso(group, edges, star_hashes);
            if (!local_result.empty()) {
                if (result.empty()) {
                    result = local_result;
                } else {
                    return {};
                }
            }
        }
    }
    return result;
}

static void check_constellation(const std::vector<Edge> &edges) {
    auto star_to_root = unite_tree(edges);
    int global_root = -1;
    if (star_to_root.size() > 50) {
        throw std::invalid_argument("constellation");
    }
    for (const auto &[star, _] : star_to_root) {
        if (star >= static_cast<int>(star_to_root.size()) || star < 0) {
            throw std::invalid_argument("constellation");
        }
        int root = get_root(star, star_to_root);
        if (global_root == -1) {
            global_root = root;
        } else if (root != global_root) {
            throw std::invalid_argument("constellation");
        }
    }
}

static void distribute_constellations(const std::vector<std::string> &star_hashes,
               std::vector<std::vector<Edge>> &star_groups,
               std::vector<int> &star_id_to_group_id) {
    for (std::size_t i = 0; i < star_hashes.size(); ++i) {
        if (star_id_to_group_id[i] == -1) {
            star_groups.push_back(std::vector<Edge>());
            star_id_to_group_id[i] = static_cast<int>(star_groups.size() - 1);
        }
        for (std::size_t j = i + 1; j < star_hashes.size(); ++j) {
            double dist;
            switch (galactic_identifier_engage(star_hashes[i].c_str(),
                                               star_hashes[j].c_str(), &dist)) {
                case CGalacticErrorCode_Ok:
                    star_id_to_group_id[j] = star_id_to_group_id[i];
                    star_groups[static_cast<std::size_t>(
                                    star_id_to_group_id[i])]
                        .push_back(Edge(static_cast<int>(i),
                                        static_cast<int>(j), dist));
                    break;
                case CGalacticErrorCode_NoResult:
                    break;
                case CGalacticErrorCode_InvalidStarIdentifier:
                    throw std::invalid_argument("Invalid star identifier");
                case CGalacticErrorCode_ModuleOverheat:
                case CGalacticErrorCode_InvalidStateError:
                    throw std::logic_error("invalid state");
            }
        }
    }
}

static std::vector<std::string> are_iso(const std::vector<Edge> &c, const std::vector<Edge> &edges, const std::vector<std::string> &star_hashes) {
    assert(c.size() == edges.size());
    Graph c_graph = transform_to_graph(c);
    Graph edges_graph = transform_to_graph(edges);
    std::vector<int> c_centroids;
    std::vector<int> edges_centroids;
    find_centroids(c_graph, c_centroids);
    find_centroids(edges_graph, edges_centroids);
    std::vector<std::string> result;
    for (const auto &c_start : c_centroids) {
        result = are_iso(c_graph, edges_graph, c_start, edges_centroids[0], star_hashes);
        if (!result.empty()) {
            return result;
        }
    }
    return result;
}

static Graph transform_to_graph(const std::vector<Edge> &edges) {
    Graph graph;
    std::set<double> sorted_edges;
    for (std::size_t i = 0; i < edges.size(); ++i) {
        double distance = edges[i].get_distance();
        if (distance < 0) {
            throw std::invalid_argument("distance < 0");
        }
        sorted_edges.insert(distance);
    }
    std::unordered_map<double, int> edge_rank;
    int dist_id = 0;
    for (auto dist : sorted_edges) {
        edge_rank[dist] = dist_id++;
    }
    for (std::size_t i = 0; i < edges.size(); ++i) {
        int from = edges[i].get_from();
        int to = edges[i].get_to();
        int rank = edge_rank[edges[i].get_distance()];
        graph[from].push_back(std::make_tuple(to, rank));
        graph[to].push_back(std::make_tuple(from, rank));
    }
    if (graph.size() != edges.size() + 1) {
        throw std::invalid_argument("not a tree");
    }
    return graph;
}

struct StackItemIso {
    std::size_t id;
    int example_vertex;
    int to_find_vertex;

    StackItemIso(std::size_t id, int example_vertex, int to_find_vertex) : id(id), example_vertex(example_vertex), to_find_vertex(to_find_vertex) {}
};

using StackIso = std::vector<StackItemIso>;

static std::vector<std::string> are_iso(Graph &example, Graph &to_find, int example_start, int to_find_start, const std::vector<std::string> &star_hashes) {
    assert(example.size() == to_find.size());
    std::vector<std::string> result(example.size());
    std::map<std::vector<std::pair<int, int>>, int> hashes;
    std::unordered_map<int, std::vector<std::tuple<int, int, int>>> example_children;
    std::unordered_map<int, std::vector<std::tuple<int, int, int>>> to_find_children;
    int example_hash = get_hash(example, example_start, hashes, example_children);
    int to_find_hash = get_hash(to_find, to_find_start, hashes, to_find_children);
    if (example_hash != to_find_hash) {
        return {};
    }
    StackIso s { StackItemIso(first_id, example_start, to_find_start) };
    while (!s.empty()) {
        auto &item = s.back();
        ++item.id;
        if (item.id == example_children[item.example_vertex].size()) {
            result[static_cast<std::size_t>(item.to_find_vertex)] = star_hashes[static_cast<std::size_t>(item.example_vertex)];
            s.pop_back();
            continue;
        }
        int new_example_vertex = std::get<2>(example_children[item.example_vertex][item.id]);
        int new_to_find_vertex = std::get<2>(to_find_children[item.to_find_vertex][item.id]);
        s.push_back(StackItemIso(first_id, new_example_vertex, new_to_find_vertex));
    }
    return result;
}

struct StackGetHashItem {
    std::size_t id;
    int vertex;
    int previous_vertex;
    int weight;

    StackGetHashItem(std::size_t id, int vertex, int weight, int previous_vertex = -1) : id(id), vertex(vertex), previous_vertex(previous_vertex), weight(weight) {}
};

using StackGetHash = std::vector<StackGetHashItem>;

static int get_hash(Graph &g, int start, std::map<std::vector<std::pair<int, int>>, int> &hashes, std::unordered_map<int, std::vector<std::tuple<int, int, int>>> &sorted_children) {
    StackGetHash s { StackGetHashItem(first_id, start, -1) };
    int hash = 0;
    while (!s.empty()) {
        auto &item = s.back();
        ++item.id;
        if (item.id == g[item.vertex].size()) {
            std::ranges::sort(sorted_children[item.vertex]);
            hash = get_hash(sorted_children[item.vertex], hashes);
            if (item.previous_vertex >= 0) {
                sorted_children[item.previous_vertex].push_back(std::make_tuple(item.weight, hash, item.vertex));
            }
            s.pop_back();
            continue;
        }
        int new_vertex = std::get<0>(g[item.vertex][item.id]);
        int new_weight = std::get<1>(g[item.vertex][item.id]);
        if (new_vertex != item.previous_vertex) {
            s.push_back(StackGetHashItem(first_id, new_vertex,  new_weight, item.vertex));
        }
    }
    return hash;
}

static int get_hash(const std::vector<std::tuple<int, int, int>> &cs, std::map<std::vector<std::pair<int, int>>, int> &hashes) {
    std::vector<std::pair<int, int>> key;
    key.reserve(cs.size());
    for (const auto& [weight, hash, vertex] : cs) {
        key.emplace_back(weight, hash);
    }
    const auto [it, inserted] = hashes.try_emplace(key, static_cast<int>(hashes.size()));
    return it->second;
}

struct StackItemCentroids {
    std::size_t id;
    int vertex;
    int previous_vertex;
    std::size_t child_size = 0;
    bool is_ok = true;

    StackItemCentroids(std::size_t id, int vertex, int previous_vertex = -1) : id(id), vertex(vertex), previous_vertex(previous_vertex) {}
};

using StackCentroids = std::vector<StackItemCentroids>;

static void find_centroids(Graph &g, std::vector<int> &centroids) {
    StackCentroids s {StackItemCentroids(first_id, g.begin()->first)};
    std::size_t subtree_size;
    while (!s.empty()) {
        auto &item = s.back();
        ++item.id;
        if (item.id == g[item.vertex].size()) {
            if (g.size() - item.child_size - 1 > g.size() / 2) {
                item.is_ok = false;
            }
            subtree_size = item.child_size + 1;
            if (item.is_ok) {
                centroids.push_back(item.vertex);
            }
            s.pop_back();
            if (!s.empty()) {
                auto &prev_item = s.back();
                if (subtree_size > g.size() / 2) {
                    prev_item.is_ok = false;
                }
                prev_item.child_size += subtree_size;
            }
            continue;
        }
        int new_vertex = std::get<0>(g[item.vertex][item.id]);
        if (new_vertex != item.previous_vertex) {
            s.push_back(StackItemCentroids(first_id, new_vertex, item.vertex));
        }
    }
}

static std::unordered_map<int, int> leave_tree(std::vector<Edge> &star_groups) {
    std::unordered_map<int, int> star_to_root;
    std::unordered_map<int, int> root_to_size;
    std::size_t free_i = 0;
    for (std::size_t i = 0; i < star_groups.size(); ++i) {
        auto &edge = star_groups[i];
        int root1 = get_root(edge.get_from(), star_to_root);
        int root2 = get_root(edge.get_to(), star_to_root);
        if (root1 != root2) {
            unite(root1, root2, star_to_root, root_to_size);
            if (free_i != i) {
                std::swap(star_groups[free_i], star_groups[i]);
            }
            ++free_i;
        }
    }
    star_groups.resize(free_i);
    return star_to_root;
}

static std::unordered_map<int, int> unite_tree(const std::vector<Edge> &star_groups) {
    std::unordered_map<int, int> star_to_root;
    std::unordered_map<int, int> root_to_size;
    for (std::size_t i = 0; i < star_groups.size(); ++i) {
        auto &edge = star_groups[i];
        int root1 = get_root(edge.get_from(), star_to_root);
        int root2 = get_root(edge.get_to(), star_to_root);
        if (root1 != root2) {
            unite(root1, root2, star_to_root, root_to_size);
        }
    }
    return star_to_root;
}

static void unite(int root1, int root2, std::unordered_map<int, int> &star_to_root, std::unordered_map<int, int> &root_to_size) {
    if (root_to_size[root1] > root_to_size[root2]) {
        std::swap(root1, root2);
    }
    star_to_root[root1] = root2;
    root_to_size[root2] += root_to_size[root1];
}

static int get_root(int star_id, std::unordered_map<int, int> &star_to_root) {
    int initial_root = star_id;
    int root = star_id;
    while (get_or_default(star_to_root, root) != -1) {
        root = star_to_root[root];
    }
    while (star_to_root[initial_root] != -1) {
        star_to_root[initial_root] = root;
        initial_root = star_to_root[initial_root];
    }
    return root;
}

static int get_or_default(std::unordered_map<int, int> &map, int star_id, int default_value) {
    if (map.find(star_id) == map.end()) {
        return map[star_id] = default_value;
    }
    return map[star_id];
}
