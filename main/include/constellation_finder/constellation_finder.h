#ifndef CONSTELLATION_FINDER_H
#define CONSTELLATION_FINDER_H

#include <string>
#include <vector>

#include "constellation_finder/constellation.h"

std::vector<std::string> solve_constellation_finder(
    const std::vector<std::string> &star_hashes, Constellation &c);

#endif // CONSTELLATION_FINDER_H
