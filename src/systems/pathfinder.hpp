#pragma once

#include <limits>
#include <utility>

// Result type stored per cell after a BFS solve.
struct Node {
    int distance = std::numeric_limits<int>::max();
    std::pair<int, int> predecessor = {-1, -1};
};
