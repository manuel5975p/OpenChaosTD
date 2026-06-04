#pragma once

#include <world/map.hpp>
#include <world/tower.hpp>
#include <world/enemy.hpp>
#include <world/attack.hpp>
#include <engine/lib/dense_slotmap.hpp>
#include <world/vfx_effect.hpp>
#include <vector>

class JsonStore;

struct GameData {
    // Gameplay config — loaded from gameplay.json, not reset between games
    int m_startingLives = 20;
    int m_startingGold = 150;
    float m_sellRefundRate = 0.5f;
    int m_totalWaves = 0;          // 0 = endless, >0 = victory after this many waves
    float m_autoSpawnDelay = 3.0f; // seconds between waves when auto-spawn is on

    int m_lives = 20;
    int m_gold = 150;
    bool m_victory = false;

    // Wave state
    int   m_waveNumber = 0;
    bool  m_waveActive = false;
    float m_waveTimer = 0.0f;  // seconds elapsed since wave started
    Map m_map;
    DenseSlotMap<Tower> m_towers;
    DenseSlotMap<Enemy> m_enemies;
    std::vector<AttackPayload> m_payloads;
    std::vector<VfxEffect> m_vfx;

    void Load(JsonStore& jsonio);
    void Reset();
};
