#pragma once

#include <world/map.hpp>
#include <world/tower.hpp>
#include <world/enemy.hpp>
#include <world/attack.hpp>
#include <engine/lib/dense_slotmap.hpp>
#include <vector>

class FileStore;

struct GameData {
    // Gameplay config — loaded from gameplay.json, not reset between games
    int m_startingLives = 20;
    int m_startingGold = 150;
    float m_sellRefundRate = 0.5f;
    float m_autoSpawnDelay = 3.0f; // seconds between waves when auto-spawn is on

    int m_lives = 20;
    int m_gold = 150;
    bool m_victory = false;

    // Wave state
    int   m_waveNumber = 0;
    bool  m_waveActive = false;
    Map m_map;
    DenseSlotMap<Tower> m_towers;
    DenseSlotMap<Enemy> m_enemies;
    std::vector<Attack> m_attacks;

    // dataDir is the active datapack's data directory (relative to the project root).
    void Load(FileStore& fileStore, const std::string& dataDir);
    void Reset();
};
