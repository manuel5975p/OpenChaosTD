#pragma once

#include <string>

#include <world/tower.hpp>
#include <engine/lib/dense_slotmap.hpp>

enum class TileType {
    Grass,
    Rock,
    Core,
    Nest,
    Buff
};

// A terrain stat modifier applied to whatever tower is built on the tile. Reuses the engine-wide
// key->delta convention (see Tower::PatchStats / TowerUpgrade): an inactive modifier (empty key)
// leaves the tower untouched, so ordinary tiles behave exactly as before.
struct TileModifier {
    std::string m_statKey;   // empty = no modifier
    float m_value = 0.0f;
    bool m_mul = false;      // false = additive, true = multiplicative
    bool Active() const { return !m_statKey.empty(); }
};

struct Tile{
    TileType m_type = TileType::Grass;
    bool m_walkable = true;
    bool m_buildable = true;

    TileModifier m_modifier; // terrain buff applied to a tower placed here (inactive by default)

    DenseSlotMap<Tower>::Key m_towerKey = DenseSlotMap<Tower>::INVALID_KEY;
};