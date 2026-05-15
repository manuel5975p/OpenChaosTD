#pragma once

#include <world/game_data.hpp>
#include <systems/world_system.hpp>
#include <factory/enemy_factory.hpp>

class WaveManager {
public:
    // Advance wave state: tick timer, detect wave end, trigger auto-spawn
    void Update(float dt, GameData& data, WorldSystem& worldSystem, EnemyFactory& enemyFactory);

    // Start the next wave; no-op if one is already active
    void StartWave(GameData& data, WorldSystem& worldSystem, EnemyFactory& enemyFactory);

    void ToggleAutoSpawn() { m_autoSpawn = !m_autoSpawn; }
    bool IsAutoSpawn() const { return m_autoSpawn; }

private:
    // Spawns enemies for one wave — replace with data-driven content when wave definitions exist
    void SpawnWave(GameData& data, WorldSystem& worldSystem, EnemyFactory& enemyFactory);

    bool m_autoSpawn = false;
};
