#include <world/map.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <queue>
#include <vector>

// --- Pathfinding internals ---------------------------------------------------
// Graph and BFS are local to this file; nothing outside needs to know about them.

namespace {

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

// BFS from start toward all reachable nodes.
// Each cell in the returned grid holds the shortest distance and predecessor.
Grid2D<Node> BfsSolve(std::pair<int, int> start, const Graph& graph) {
    Grid2D<Node> mesh;
    if (!graph.InBounds(start)) {
        std::cerr << "BfsSolve: start node out of bounds\n";
        return mesh;
    }

    mesh.Resize(graph.Width(), graph.Height());

    std::queue<std::pair<int, int>> q;
    mesh.Get(start.first, start.second).distance = 0;
    mesh.Get(start.first, start.second).predecessor = start;
    q.push(start);

    while (!q.empty()) {
        std::pair<int, int> u = q.front();
        q.pop();
        for (const Edge& edge : graph.Neighbors(u)) {
            std::pair<int, int> v = edge.target;
            if (mesh.Get(v.first, v.second).distance == std::numeric_limits<int>::max()) {
                mesh.Get(v.first, v.second).distance = mesh.Get(u.first, u.second).distance + 1;
                mesh.Get(v.first, v.second).predecessor = u;
                q.push(v);
            }
        }
    }
    return mesh;
}

} // namespace

// --- Map methods -------------------------------------------------------------

Vector2 Map::TileToWorld(int x, int y) const {
    return {
        static_cast<float>(x * m_tileSize),
        static_cast<float>(y * m_tileSize)
    };
}

bool Map::WorldToTile(Vector2 worldPos, int& outX, int& outY) const {
    outX = static_cast<int>(std::floor(worldPos.x / m_tileSize));
    outY = static_cast<int>(std::floor(worldPos.y / m_tileSize));
    return m_grid.InBounds(outX, outY);
}

void Map::Create(int cols, int rows) {
    m_grid.Resize(cols, rows);
}

void Map::SetCore(int cols, int rows) {
    m_core = {cols, rows};
    m_grid.Get(cols, rows).m_type = TileType::Core;
    m_grid.Get(cols, rows).m_buildable = false;
    m_grid.Get(cols, rows).m_walkable = true;
}

void Map::AddNest(int cols, int rows) {
    for (auto& nest : m_nests) {
        if (nest.first == cols && nest.second == rows)
            return; // already a nest
    }

    m_nests.push_back({cols, rows});
    m_grid.Get(cols, rows).m_type = TileType::Nest;
    m_grid.Get(cols, rows).m_buildable = false;
    m_grid.Get(cols, rows).m_walkable = true;
    m_paths.push_back({}); // reserve slot for this nest's path
}

void Map::BuildPathMesh() {
    static const int dx4[] = { 1, -1,  0,  0 };
    static const int dy4[] = { 0,  0,  1, -1 };

    int width  = m_grid.GetWidth();
    int height = m_grid.GetHeight();

    Graph graph;
    graph.Resize(width, height);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (!m_grid.Get(x, y).m_walkable) continue;
            for (int d = 0; d < 4; ++d) {
                int nx = x + dx4[d];
                int ny = y + dy4[d];
                if (m_grid.InBounds(nx, ny) && m_grid.Get(nx, ny).m_walkable)
                    graph.AddEdge({x, y}, {nx, ny});
            }
        }
    }

    m_pathMesh = BfsSolve({m_core.first, m_core.second}, graph);

    ConstructPaths();
}

bool Map::ValidatePathMesh() {
    for (auto& nest : m_nests) {
        if (m_pathMesh.Get(nest.first, nest.second).distance == std::numeric_limits<int>::max())
            return false;
    }
    return true;
}

void Map::ConstructPaths() {
    float half = static_cast<float>(m_tileSize) / 2.0f;

    for (size_t i = 0; i < m_paths.size(); i++) {
        m_paths[i].clear();

        std::pair<int, int> nest = m_nests[i];
        if (!m_pathMesh.InBounds(nest.first, nest.second) ||
            m_pathMesh.Get(nest.first, nest.second).distance == std::numeric_limits<int>::max())
            continue;

        // Walk predecessor chain from nest back to core, then reverse for spawn-to-core order
        std::vector<std::pair<int, int>> nodes;
        std::pair<int, int> current = nest;
        while (m_pathMesh.Get(current.first, current.second).predecessor != current) {
            nodes.push_back(current);
            current = m_pathMesh.Get(current.first, current.second).predecessor;
        }
        nodes.push_back(current); // core itself
        std::reverse(nodes.begin(), nodes.end());

        for (auto& node : nodes) {
            m_paths[i].push_back({
                node.first  * static_cast<float>(m_tileSize) + half,
                node.second * static_cast<float>(m_tileSize) + half
            });
        }
    }
}
