#pragma once

#include <world/map.hpp>
#include <world/tower.hpp>
#include <world/enemy.hpp>
#include <world/attack.hpp>
#include <lib/dense_slotmap.hpp>
#include <world/vfx_effect.hpp>
#include <vector>

class JsonIO;

struct GameData {
    // Gameplay config — loaded from gameplay.json, not reset between games
    int startingLives = 20;
    int startingGold = 150;
    float sellRefundRate = 0.5f;
    int totalWaves = 0;          // 0 = endless, >0 = victory after this many waves
    float autoSpawnDelay = 3.0f; // seconds between waves when auto-spawn is on

    int lives = 20;
    int gold = 150;
    bool victory = false;

    // Wave state
    int   waveNumber = 0;
    bool  waveActive = false;
    float waveTimer  = 0.0f;  // seconds elapsed since wave started
    Map map;
    DenseSlotMap<Tower> towers;
    DenseSlotMap<Enemy> enemies;
    std::vector<AttackPayload> m_payloads;
    std::vector<VfxEffect> m_vfx;

    void Load(JsonIO& jsonio);
    void Reset();
};
