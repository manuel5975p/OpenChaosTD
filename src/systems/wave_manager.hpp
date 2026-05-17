#pragma once

#include <world/game_data.hpp>
#include <systems/world_system.hpp>
#include <factory/enemy_factory.hpp>
#include <core/jsonio.hpp>
#include <string>
#include <vector>

class WaveManager {
public:
    void Load(JsonIO& jsonio);

    // Advance wave state: tick timer, process spawn queue, detect wave end, trigger auto-spawn
    void Update(float dt, GameData& data, WorldSystem& worldSystem, EnemyFactory& enemyFactory);

    // Start the next wave; no-op if one is already active
    void StartWave(GameData& data);

    void ToggleAutoSpawn() { m_autoSpawn = !m_autoSpawn; }
    bool IsAutoSpawn() const { return m_autoSpawn; }

private:
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

    // A single resolved spawn event in the active wave's queue
    struct PendingSpawn {
        std::string type;
        int nest;
        float time; // seconds after wave start when this enemy should spawn
    };

    // Expands a WaveDef into a sorted PendingSpawn list.
    // countMultiplier scales all counts (used for endless waves beyond defined set).
    void BuildSpawnQueue(const WaveDef& def, int nestCount, int countMultiplier);

    std::vector<WaveDef> m_waveDefs;

    std::vector<PendingSpawn> m_pendingSpawns;
    int m_nextSpawn = 0;     // index of the next unprocessed pending spawn
    float m_elapsed = 0.0f;  // seconds elapsed since the current wave started

    bool m_autoSpawn = false;
    float m_autoSpawnTimer = 0.0f;
};
