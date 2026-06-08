#include <systems/pathfinder.hpp>

#include <cassert>
#include <iostream>
#include <limits>
#include <queue>
#include <vector>

// --- Pathfinding internals ---------------------------------------------------
// The adjacency graph and BFS are local to this file; nothing outside needs them.

namespace {

// 4-neighbour offsets (east, west, south, north).
constexpr int kDx4[] = { 1, -1,  0,  0 };
constexpr int kDy4[] = { 0,  0,  1, -1 };

struct Edge {
    std::pair<int, int> target;
};

class Graph {
    Grid2D<std::vector<Edge>> m_adj;
public:
    void Resize(int w, int h) { m_adj.Resize(w, h); }

    bool InBounds(std::pair<int, int> t) const {
        return m_adj.InBounds(t.first, t.second);
    }

    void AddEdge(std::pair<int, int> from, std::pair<int, int> to) {
        assert(m_adj.InBounds(from.first, from.second));
        assert(m_adj.InBounds(to.first,   to.second));
        m_adj.Get(from.first, from.second).push_back({to});
    }

    const std::vector<Edge>& Neighbors(std::pair<int, int> pos) const {
        return m_adj.Get(pos.first, pos.second);
    }

    int Width()  const { return m_adj.GetWidth();  }
    int Height() const { return m_adj.GetHeight(); }
};

// Build a 4-connected graph over every walkable cell of the mask.
Graph BuildGraph(const WalkableMask& walkable) {
    int width  = walkable.GetWidth();
    int height = walkable.GetHeight();

    Graph graph;
    graph.Resize(width, height);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (!walkable.Get(x, y)) continue;
            for (int d = 0; d < 4; ++d) {
                int nx = x + kDx4[d];
                int ny = y + kDy4[d];
                if (walkable.InBounds(nx, ny) && walkable.Get(nx, ny))
                    graph.AddEdge({x, y}, {nx, ny});
            }
        }
    }
    return graph;
}

// BFS from the goal toward all reachable nodes. Each cell of the returned grid holds the
// shortest distance and the predecessor on the way back to the goal.
Grid2D<Node> BfsSolve(std::pair<int, int> start, const Graph& graph) {
    Grid2D<Node> mesh;
    if (!graph.InBounds(start)) {
        std::cerr << "Pathfinder: start node out of bounds\n";
        return mesh;
    }

    mesh.Resize(graph.Width(), graph.Height());

    std::queue<std::pair<int, int>> q;
    mesh.Get(start.first, start.second).m_distance = 0;
    mesh.Get(start.first, start.second).m_predecessor = start;
    q.push(start);

    while (!q.empty()) {
        std::pair<int, int> u = q.front();
        q.pop();
        for (const Edge& edge : graph.Neighbors(u)) {
            std::pair<int, int> v = edge.target;
            if (mesh.Get(v.first, v.second).m_distance == std::numeric_limits<int>::max()) {
                mesh.Get(v.first, v.second).m_distance = mesh.Get(u.first, u.second).m_distance + 1;
                mesh.Get(v.first, v.second).m_predecessor = u;
                q.push(v);
            }
        }
    }
    return mesh;
}

} // namespace

Grid2D<Node> Pathfinder::Solve(const WalkableMask& walkable, std::pair<int, int> goal) {
    return BfsSolve(goal, BuildGraph(walkable));
}
