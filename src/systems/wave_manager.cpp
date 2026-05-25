#include <systems/wave_manager.hpp>
#include <algorithm>
#include <iostream>

void WaveManager::Load(JsonStore& jsonio) {
    if (!jsonio.Exists("config/waves.json")) {
        std::cerr << "WaveManager: data/waves.json not found\n";
        return;
    }

    auto json = jsonio.Load("config/waves.json");
    if (!json.contains("waves")) return;

    for (const auto& wave : json["waves"]) {
        WaveDef def;
        if (!wave.contains("spawns")) { m_waveDefs.push_back(def); continue; }

        for (const auto& s : wave["spawns"]) {
            SpawnGroup g;
            g.enemyType = s["enemy"];
            g.count = s.value("count", 1);
            g.nest = s.value("nest", -1);
            g.interval = s.value("interval", 1.0f);
            g.delay = s.value("delay", 0.0f);
            def.groups.push_back(g);
        }
        m_waveDefs.push_back(std::move(def));
    }

    std::cout << "WaveManager: loaded " << m_waveDefs.size() << " wave definitions\n";
}

void WaveManager::BuildSpawnQueue(const WaveDef& def, int nestCount, int countMultiplier) {
    m_pendingSpawns.clear();
    m_nextSpawn = 0;
    m_elapsed = 0.0f;

    int nestIdx = 0;
    for (const auto& grp : def.groups) {
        int count = grp.count * countMultiplier;
        for (int i = 0; i < count; i++) {
            PendingSpawn ps;
            ps.type = grp.enemyType;
            ps.nest = (grp.nest >= 0) ? grp.nest : (nestIdx++ % nestCount);
            ps.time = grp.delay + i * grp.interval;
            m_pendingSpawns.push_back(ps);
        }
    }

    std::sort(m_pendingSpawns.begin(), m_pendingSpawns.end(),
        [](const PendingSpawn& a, const PendingSpawn& b) { return a.time < b.time; });
}

void WaveManager::Update(float dt, GameData& data, WorldSystem& worldSystem, EnemyFactory& enemyFactory) {
    if (data.waveActive) {
        data.waveTimer += dt;
        m_elapsed += dt;

        // Fire any spawns whose scheduled time has arrived
        while (m_nextSpawn < static_cast<int>(m_pendingSpawns.size())
               && m_pendingSpawns[m_nextSpawn].time <= m_elapsed) {
            const auto& ps = m_pendingSpawns[m_nextSpawn++];
            if (enemyFactory.Has(ps.type))
                worldSystem.SpawnEnemy(ps.nest, enemyFactory.Create(ps.type), data);
        }

        // Wave ends once the spawn queue is exhausted and all enemies are cleared
        bool queueDone = m_nextSpawn >= static_cast<int>(m_pendingSpawns.size());
        if (queueDone && data.enemies.Size() == 0) {
            data.waveActive = false;
            m_autoSpawnTimer = 0.0f;

            if (data.totalWaves > 0 && data.waveNumber >= data.totalWaves)
                data.victory = true;
        }
        return;
    }

    // Auto-spawn: start the next wave after the configured delay
    if (m_autoSpawn && data.waveNumber > 0 && !data.victory) {
        m_autoSpawnTimer += dt;
        if (m_autoSpawnTimer >= data.autoSpawnDelay)
            StartWave(data);
    }
}

void WaveManager::StartWave(GameData& data) {
    if (data.waveActive) return;

    data.waveNumber++;
    data.waveActive = true;
    data.waveTimer = 0.0f;

    if (m_waveDefs.empty()) return;

    int waveIdx = data.waveNumber - 1;
    int defCount = static_cast<int>(m_waveDefs.size());
    // Clamp to last defined wave; scale counts for waves beyond the defined set
    int clampedIdx = std::min(waveIdx, defCount - 1);
    int multiplier = (waveIdx >= defCount) ? (waveIdx / defCount) + 1 : 1;
    int nestCount = static_cast<int>(data.map.GetNests().size());

    BuildSpawnQueue(m_waveDefs[clampedIdx], nestCount, multiplier);
}
