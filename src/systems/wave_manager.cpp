#include <systems/wave_manager.hpp>

void WaveManager::Update(float dt, GameData& data, WorldSystem& worldSystem, EnemyFactory& enemyFactory) {
    if (data.waveActive) {
        data.waveTimer += dt;

        // End the wave once all enemies are cleared
        if (data.enemies.Size() == 0) {
            data.waveActive = false;
            m_autoSpawnTimer = 0.0f;

            // Victory: all waves completed (totalWaves == 0 means endless)
            if (data.totalWaves > 0 && data.waveNumber >= data.totalWaves)
                data.victory = true;
        }
        return;
    }

    // Auto-spawn: start the next wave after the configured delay
    if (m_autoSpawn && data.waveNumber > 0 && !data.victory) {
        m_autoSpawnTimer += dt;
        if (m_autoSpawnTimer >= data.autoSpawnDelay)
            StartWave(data, worldSystem, enemyFactory);
    }
}

void WaveManager::StartWave(GameData& data, WorldSystem& worldSystem, EnemyFactory& enemyFactory) {
    if (data.waveActive) return;

    data.waveNumber++;
    data.waveActive = true;
    data.waveTimer  = 0.0f;
    SpawnWave(data, worldSystem, enemyFactory);
}

void WaveManager::SpawnWave(GameData& data, WorldSystem& worldSystem, EnemyFactory& enemyFactory) {
    worldSystem.SpawnEnemy(0, enemyFactory.Create("voidno"), data);
    worldSystem.SpawnEnemy(1, enemyFactory.Create("voidno"), data);
    worldSystem.SpawnEnemy(2, enemyFactory.Create("voidno"), data);
}
