#ifndef CONSTELLATION_H
#define CONSTELLATION_H

#include <vector>
#include "constellation_finder/edge.h"

class Constellation {
public:
    explicit Constellation(const std::vector<Edge> &edges): _edges(edges) {}
    std::vector<Edge> &get_edges() { return _edges; }
private:
    std::vector<Edge> _edges;
};

#endif // CONSTELLATION_H
