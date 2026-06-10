#pragma once

// JSON (de)serialization for the persisted game state. Lives in the game layer so the
// engine containers (Grid2D, DenseSlotMap) stay free of any json dependency: they expose
// generic raw accessors and we reach in through non-intrusive ADL to_json/from_json here.

#include <vector>
#include <string>

#include <raylib.h>
#include <nlohmann/json.hpp>

#include <world/tile.hpp>          // Tile + DenseSlotMap<Tower>::Key
#include <world/tower.hpp>         // Tower (move-only) — serialized by value below
#include <engine/lib/grid2d.hpp>
#include <engine/lib/dense_slotmap.hpp>

class Map;
class TowerFactory;

// --- ADL serializers for value types (global namespace so ADL finds them) ---

inline void to_json(nlohmann::json& j, const Vector2& v) {
    j = nlohmann::json{{"x", v.x}, {"y", v.y}};
}
inline void from_json(const nlohmann::json& j, Vector2& v) {
    v.x = j.at("x").get<float>();
    v.y = j.at("y").get<float>();
}

inline void to_json(nlohmann::json& j, const TileModifier& m) {
    j = nlohmann::json{{"statKey", m.m_statKey}, {"value", m.m_value}, {"mul", m.m_mul}};
}
inline void from_json(const nlohmann::json& j, TileModifier& m) {
    m.m_statKey = j.value("statKey", std::string{});
    m.m_value   = j.value("value", 0.0f);
    m.m_mul     = j.value("mul", false);
}

inline void to_json(nlohmann::json& j, const Tile& t) {
    j = nlohmann::json{
        {"type", static_cast<int>(t.m_type)},
        {"walkable", t.m_walkable},
        {"buildable", t.m_buildable},
        {"modifier", t.m_modifier},
        {"towerKey", {{"index", t.m_towerKey.index}, {"generation", t.m_towerKey.generation}}},
    };
}
inline void from_json(const nlohmann::json& j, Tile& t) {
    t.m_type      = static_cast<TileType>(j.value("type", 0));
    t.m_walkable  = j.value("walkable", true);
    t.m_buildable = j.value("buildable", true);
    t.m_modifier  = j.value("modifier", TileModifier{});
    const auto& k = j.at("towerKey");
    t.m_towerKey  = { k.at("index").get<uint32_t>(), k.at("generation").get<uint32_t>() };
}

// Grid2D<T> — width/height plus the flat row-major cell vector. Generic over any
// serializable cell type; never includes game logic.
template<class T>
void to_json(nlohmann::json& j, const Grid2D<T>& g) {
    j = nlohmann::json{{"width", g.GetWidth()}, {"height", g.GetHeight()}, {"data", g.GetVector()}};
}
template<class T>
void from_json(const nlohmann::json& j, Grid2D<T>& g) {
    int w = j.at("width").get<int>();
    int h = j.at("height").get<int>();
    auto data = j.at("data").template get<std::vector<T>>();
    if (static_cast<size_t>(w) * static_cast<size_t>(h) != data.size())
        throw std::runtime_error("Grid2D: data size does not match width*height");
    g.Resize(w, h);
    g.GetVector() = std::move(data);
}

// --- Tower / DenseSlotMap<Tower> ---
// Towers are serialized identity-only (metadata); their polymorphic module lists are
// rebuilt through TowerFactory on load, never persisted. The slotmap's sparse bookkeeping
// is persisted verbatim so DenseSlotMap<Tower>::Key handles (e.g. Tile::m_towerKey) survive.

inline nlohmann::json SaveTower(const Tower& t) {
    return nlohmann::json{
        {"name", t.m_name},
        {"position", t.m_position},
        {"level", t.m_level},
        {"cooldown", t.m_cooldown},
        {"cost", t.m_cost},
    };
}

inline nlohmann::json SaveTowers(const DenseSlotMap<Tower>& towers) {
    nlohmann::json slots = nlohmann::json::array();
    for (const auto& s : towers.RawSlots())
        slots.push_back({{"generation", s.generation}, {"dense", s.dense_index}, {"occupied", s.occupied}});

    nlohmann::json values = nlohmann::json::array();
    for (const auto& t : towers.RawValues())
        values.push_back(SaveTower(t));

    return nlohmann::json{
        {"slots", slots},
        {"erase", towers.RawErase()},
        {"freeList", towers.RawFreeList()},
        {"values", values},
    };
}

// Rebuild the tower slotmap from JSON. Returns false (leaving `out` unspecified) on any
// structural inconsistency or unknown tower name, so a corrupt save never half-loads.
// `restoredMap` supplies the tiles whose terrain modifiers must be re-applied to towers.
bool LoadTowers(const nlohmann::json& j, DenseSlotMap<Tower>& out,
                const TowerFactory& factory, const Map& restoredMap);
