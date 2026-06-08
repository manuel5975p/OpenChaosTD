#pragma once

#include <limits>
#include <utility>
#include <engine/lib/grid2d.hpp>

// Per-cell result of a BFS solve: shortest hop distance to the goal and the next cell along
// that shortest path (the predecessor pointing back toward the goal).
struct Node {
    int m_distance = std::numeric_limits<int>::max();
    std::pair<int, int> m_predecessor = {-1, -1};
};

// Abstract walkability input: one cell per grid position, non-zero meaning walkable. Using an
// unsigned-char grid (rather than Grid2D<bool>, whose std::vector<bool> backing can't return a
// reference) keeps the solver free of any Tile/Map type.
using WalkableMask = Grid2D<unsigned char>;

// Breadth-first solver over an abstract walkable grid. It knows nothing about tiles or maps:
// it consumes a walkability mask plus a goal cell and returns the per-cell distance/predecessor
// mesh that flows back toward the goal.
class Pathfinder {
public:
    static Grid2D<Node> Solve(const WalkableMask& walkable, std::pair<int, int> goal);
};
