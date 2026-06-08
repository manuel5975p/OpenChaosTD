#include <world/map.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

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

void Map::SetBuff(int cols, int rows, std::string statKey, float value, bool mul) {
    Tile& tile = m_grid.Get(cols, rows);
    tile.m_type = TileType::Buff;
    tile.m_walkable = true;  // towers place normally and pathing is unaffected
    tile.m_buildable = true;
    tile.m_modifier = {std::move(statKey), value, mul};
}

void Map::BuildPathMesh() {
    // Adapt the tile grid into an abstract walkability mask, then let the Pathfinder solve on it.
    // The Pathfinder stays free of any Tile/Map knowledge.
    int width  = m_grid.GetWidth();
    int height = m_grid.GetHeight();

    WalkableMask walkable(width, height);
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            walkable.Set(x, y, m_grid.Get(x, y).m_walkable ? 1 : 0);

    m_pathMesh = Pathfinder::Solve(walkable, m_core);

    ConstructPaths();
}

bool Map::ValidatePathMesh() {
    for (auto& nest : m_nests) {
        if (m_pathMesh.Get(nest.first, nest.second).m_distance == std::numeric_limits<int>::max())
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
            m_pathMesh.Get(nest.first, nest.second).m_distance == std::numeric_limits<int>::max())
            continue;

        // Walk predecessor chain from nest back to core, then reverse for spawn-to-core order
        std::vector<std::pair<int, int>> nodes;
        std::pair<int, int> current = nest;
        while (m_pathMesh.Get(current.first, current.second).m_predecessor != current) {
            nodes.push_back(current);
            current = m_pathMesh.Get(current.first, current.second).m_predecessor;
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
