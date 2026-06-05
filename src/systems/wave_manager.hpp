#pragma once

#include <world/game_data.hpp>
#include <systems/world_system.hpp>
#include <factory/enemy_factory.hpp>
#include <engine/util/json_store.hpp>
#include <random>
#include <string>
#include <vector>

class WaveManager {
public:
    // A group of enemies sharing the same type and spawn timing within a wave
    struct SpawnGroup {
        std::string enemyType;
        int count = 1;
        int nest = -1;       // -1 = round-robin across all nests
        float interval = 1.0f; // seconds between enemies in this group
        float delay = 0.0f;    // offset from wave start before first enemy
    };

    struct WaveDef {
        std::vector<SpawnGroup> groups;
    };

    void Load(JsonStore& jsonio);

    // Advance wave state: tick timer, process spawn queue, detect wave end, trigger auto-spawn
    void Update(float dt, GameData& data, WorldSystem& worldSystem, EnemyFactory& enemyFactory);

    // Start the next wave; no-op if one is already active
    void StartWave(GameData& data);

    void ToggleAutoSpawn() { m_autoSpawn = !m_autoSpawn; }
    bool IsAutoSpawn() const { return m_autoSpawn; }

    // --- Read access for the HUD (composition of the upcoming wave, win condition, scaling) ---
    const WaveDef& GetNextWaveDef() const { return m_pendingDef; }
    int GetVictoryWave() const { return m_victoryWave; } // 0 = endless
    int GetUpgradeTier() const { return m_activeTier; }

private:
    // One enemy type the procedural generator may draw from, with its threat cost and pacing.
    struct PoolEntry {
        std::string enemy;
        int cost = 1;
        int minWave = 1;        // earliest wave this type may appear in
        float interval = 1.0f;  // seconds between spawns of this type within a wave
    };

    // A single resolved spawn event in the active wave's queue
    struct PendingSpawn {
        std::string type;
        int nest;
        float time; // seconds after wave start when this enemy should spawn
    };

    // Procedurally compose a wave from the threat budget for that wave number.
    WaveDef GenerateWave(int waveNumber);

    // Look up a pool entry by enemy name; nullptr if absent (e.g. an unlisted boss).
    const PoolEntry* FindPoolEntry(const std::string& name) const;

    // Upgrade tier applied to enemies spawned in the given wave (0 = none).
    int UpgradeTierFor(int waveNumber) const;

    // Apply the current upgrade tier to a freshly created enemy before it spawns.
    void ApplyTierUpgrades(Enemy& enemy, int tier, const EnemyFactory& enemyFactory) const;

    // Expands a WaveDef into a sorted PendingSpawn list.
    void BuildSpawnQueue(const WaveDef& def, int nestCount);

    // --- Configuration parsed from waves.json ---
    float m_baseBudget = 20.0f;
    float m_growthExponent = 1.25f;
    int m_victoryWave = 0;          // 0 = endless, >0 = victory after this wave is cleared
    int m_bossInterval = 0;         // boss wave every Nth wave; 0 disables
    std::vector<std::string> m_bossEnemies;
    int m_upgradeInterval = 0;      // upgrade tier increments every Nth wave; 0 disables
    std::vector<PoolEntry> m_enemyPool;

    // --- 1-wave lookahead: both the next wave and the one after are pre-generated ---
    WaveDef m_pendingDef;   // composition for the next wave to launch (waveNumber + 1)
    WaveDef m_lookaheadDef; // composition for the wave after that (promoted on StartWave)
    int m_activeTier = 0;   // upgrade tier of the wave currently spawning

    std::mt19937 m_rng;

    std::vector<PendingSpawn> m_pendingSpawns;
    int m_nextSpawn = 0;     // index of the next unprocessed pending spawn
    float m_elapsed = 0.0f;  // seconds elapsed since the current wave started

    bool m_autoSpawn = false;
    float m_autoSpawnTimer = 0.0f;
};
