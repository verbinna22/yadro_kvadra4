#ifndef EDGE_H
#define EDGE_H

#include <stdexcept>
class Edge {
public:
    Edge(): _from(0), _to(0), _distance(0.0) { throw std::logic_error("invalid state"); }
    Edge(int from, int to, double distance): 
        _from(from), _to(to), _distance(distance) {}
    int get_from() const { return _from; }
    int get_to() const { return _to; }
    double get_distance() const { return _distance; }
private:
    int _from;
    int _to;
    double _distance;
};

#endif // EDGE_H
