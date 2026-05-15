#include <systems/wave_manager.hpp>

void WaveManager::Update(float dt, GameData& data, WorldSystem& worldSystem, EnemyFactory& enemyFactory) {
    if (data.waveActive) {
        data.waveTimer += dt;

        // End the wave once all enemies are cleared
        if (data.enemies.Size() == 0)
            data.waveActive = false;

        return;
    }

    // Auto-spawn: immediately start the next wave after the current one ends
    if (m_autoSpawn && data.waveNumber > 0)
        StartWave(data, worldSystem, enemyFactory);
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
