#include <systems/wave_manager.hpp>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <iostream>

void WaveManager::Load(JsonStore& jsonio, const EnemyFactory& enemyFactory) {
    m_rng.seed(std::random_device{}());

    if (!jsonio.Exists("data/waves.json")) {
        std::cerr << "WaveManager: data/waves.json not found\n";
        return;
    }

    auto json = jsonio.Load("data/waves.json");

    // Budget scaling and win condition
    if (json.contains("budget")) {
        const auto& b = json["budget"];
        m_baseBudget = b.value("base_budget", m_baseBudget);
        m_growthExponent = b.value("growth_exponent", m_growthExponent);
        m_victoryWave = b.value("victory_wave", m_victoryWave);
    }

    // Periodic boss waves
    if (json.contains("boss")) {
        const auto& boss = json["boss"];
        m_bossInterval = boss.value("interval", m_bossInterval);
        if (boss.contains("boss_enemies"))
            for (const auto& name : boss["boss_enemies"])
                m_bossEnemies.push_back(name.get<std::string>());
    }

    m_upgradeInterval = json.value("upgrade_interval", m_upgradeInterval);

    // Enemy pool the generator draws from
    if (json.contains("enemy_pool")) {
        for (const auto& e : json["enemy_pool"]) {
            PoolEntry entry;
            entry.enemy = e["enemy"].get<std::string>();
            entry.cost = e.value("cost", 1);
            entry.minWave = e.value("min_wave", 1);
            entry.interval = e.value("interval", 1.0f);
            m_enemyPool.push_back(std::move(entry));
        }
    }

    // 1-wave lookahead: pre-generate the active wave (1) and its lookahead (2) before the first start.
    m_pendingDef = GenerateWave(1);
    m_lookaheadDef = GenerateWave(2);
    m_pendingBudget = BudgetForWave(1);
    m_lookaheadBudget = BudgetForWave(2);

    // Build the prototype pool previewing wave 1 (the next wave to launch).
    RebuildPreviewPrototypes(1, enemyFactory);

    std::cout << "WaveManager: loaded " << m_enemyPool.size() << " pool entries, victory_wave="
              << m_victoryWave << "\n";
}

const WaveManager::PoolEntry* WaveManager::FindPoolEntry(const std::string& name) const {
    for (const auto& e : m_enemyPool)
        if (e.enemy == name) return &e;
    return nullptr;
}

int WaveManager::UpgradeTierFor(int waveNumber) const {
    if (m_upgradeInterval <= 0) return 0;
    return waveNumber / m_upgradeInterval;
}

float WaveManager::BudgetForWave(int waveNumber) const {
    return m_baseBudget * std::pow(static_cast<float>(waveNumber), m_growthExponent);
}

WaveManager::WaveDef WaveManager::GenerateWave(int waveNumber) {
    WaveDef def;
    if (m_enemyPool.empty()) return def;

    float budget = BudgetForWave(waveNumber);

    // Selection pool: types unlocked by this wave and not reserved as bosses.
    std::vector<const PoolEntry*> pool;
    for (const auto& e : m_enemyPool) {
        if (e.minWave > waveNumber) continue;
        if (std::find(m_bossEnemies.begin(), m_bossEnemies.end(), e.enemy) != m_bossEnemies.end())
            continue;
        pool.push_back(&e);
    }

    // Boss wave: force exactly one boss and bill its cost against the budget; the rest funds escorts.
    bool isBossWave = m_bossInterval > 0 && (waveNumber % m_bossInterval) == 0 && !m_bossEnemies.empty();
    if (isBossWave) {
        std::uniform_int_distribution<std::size_t> pick(0, m_bossEnemies.size() - 1);
        const std::string& bossName = m_bossEnemies[pick(m_rng)];
        const PoolEntry* boss = FindPoolEntry(bossName);

        SpawnGroup g;
        g.enemyType = bossName;
        g.count = 1;
        g.nest = -1;
        g.interval = boss ? boss->interval : 1.5f;
        def.groups.push_back(std::move(g));

        if (boss) budget -= static_cast<float>(boss->cost);
        if (budget < 0.0f) budget = 0.0f;
    }

    if (pool.empty()) return def; // boss-only wave (or nothing unlocked yet)

    // Cheapest unit sets the loop's termination floor.
    int cheapest = pool.front()->cost;
    for (const auto* e : pool) cheapest = std::min(cheapest, e->cost);

    // Semi-random selection: draw affordable units until the budget can't afford the cheapest.
    std::unordered_map<std::string, int> tally;
    while (budget >= static_cast<float>(cheapest)) {
        std::vector<const PoolEntry*> affordable;
        for (const auto* e : pool)
            if (static_cast<float>(e->cost) <= budget) affordable.push_back(e);
        if (affordable.empty()) break;

        std::uniform_int_distribution<std::size_t> pick(0, affordable.size() - 1);
        const PoolEntry* chosen = affordable[pick(m_rng)];
        tally[chosen->enemy]++;
        budget -= static_cast<float>(chosen->cost);
    }

    // Group identical selections into one spawn group each.
    for (const auto& [name, count] : tally) {
        const PoolEntry* e = FindPoolEntry(name);
        SpawnGroup g;
        g.enemyType = name;
        g.count = count;
        g.nest = -1;
        g.interval = e ? e->interval : 1.0f;
        g.delay = 0.0f;
        def.groups.push_back(std::move(g));
    }

    return def;
}

void WaveManager::ApplyTierUpgrades(Enemy& enemy, int tier, const EnemyFactory& enemyFactory) const {
    if (tier <= 0 || !enemy.m_upgrade) return;

    // The enemy defines a single upgrade option; re-apply it `tier` times so each upgrade tier
    // stacks one more copy of the same deltas, scaling indefinitely (endless mode).
    for (int i = 0; i < tier; i++)
        enemyFactory.ApplyUpgrade(enemy, *enemy.m_upgrade);
}

void WaveManager::RecomputeLive(Enemy& enemy) const {
    BaseStatsModule* base = enemy.GetBaseStats();
    if (!base) return;
    base->ResetLive();
    for (auto& mod : enemy.m_modules)
        mod->ContributeStats(*base);
}

void WaveManager::RebuildPreviewPrototypes(int pendingWaveNumber, const EnemyFactory& enemyFactory) {
    m_previewPrototypes.clear();
    int tier = UpgradeTierFor(pendingWaveNumber);

    // One fully-upgraded prototype per unique enemy type in the upcoming wave.
    for (const auto& grp : m_pendingDef.groups) {
        if (m_previewPrototypes.count(grp.enemyType)) continue;
        if (!enemyFactory.Has(grp.enemyType)) continue; // e.g. an unlisted type — nothing to clone
        Enemy proto = enemyFactory.Create(grp.enemyType);
        ApplyTierUpgrades(proto, tier, enemyFactory);
        RecomputeLive(proto); // refresh live speed/armor so the HUD shows upgraded values
        m_previewPrototypes.emplace(grp.enemyType, std::move(proto));
    }
}

void WaveManager::BuildSpawnQueue(const WaveDef& def, int nestCount) {
    m_pendingSpawns.clear();
    m_nextSpawn = 0;
    m_elapsed = 0.0f;

    if (nestCount <= 0) return; // no nests — nothing to spawn from, and avoids modulo-by-zero

    int nestIdx = 0;
    for (const auto& grp : def.groups) {
        for (int i = 0; i < grp.count; i++) {
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
    if (data.m_waveActive) {
        data.m_waveTimer += dt;
        m_elapsed += dt;

        // Fire any spawns whose scheduled time has arrived, cloning from the pre-upgraded prototypes
        while (m_nextSpawn < static_cast<int>(m_pendingSpawns.size())
               && m_pendingSpawns[m_nextSpawn].time <= m_elapsed) {
            const auto& ps = m_pendingSpawns[m_nextSpawn++];
            auto it = m_spawnPrototypes.find(ps.type);
            if (it == m_spawnPrototypes.end()) continue; // unknown type — no prototype to clone
            worldSystem.SpawnEnemy(ps.nest, it->second.Clone(), data);
        }

        // Wave ends once the spawn queue is exhausted and all enemies are cleared
        bool queueDone = m_nextSpawn >= static_cast<int>(m_pendingSpawns.size());
        if (queueDone && data.m_enemies.Size() == 0) {
            data.m_waveActive = false;
            m_autoSpawnTimer = 0.0f;

            // Win condition is driven entirely by waves.json (victory_wave); 0 = endless.
            if (m_victoryWave > 0 && data.m_waveNumber >= m_victoryWave)
                data.m_victory = true;
        }
        return;
    }

    // Auto-spawn: start the next wave after the configured delay
    if (m_autoSpawn && data.m_waveNumber > 0 && !data.m_victory) {
        m_autoSpawnTimer += dt;
        if (m_autoSpawnTimer >= data.m_autoSpawnDelay)
            StartWave(data, enemyFactory);
    }
}

void WaveManager::StartWave(GameData& data, const EnemyFactory& enemyFactory) {
    if (data.m_waveActive) return;

    data.m_waveNumber++;
    data.m_waveActive = true;
    data.m_waveTimer = 0.0f;

    m_activeTier = UpgradeTierFor(data.m_waveNumber);

    int nestCount = static_cast<int>(data.m_map.GetNests().size());
    BuildSpawnQueue(m_pendingDef, nestCount); // m_pendingDef holds this wave's composition

    // The preview prototypes were already upgraded to exactly this wave's tier, so promote them to
    // the active spawn set — Update clones from these without re-running the upgrade path.
    m_spawnPrototypes = std::move(m_previewPrototypes);

    // Promote the lookahead to pending and pre-generate the new lookahead (keeps one wave ahead).
    m_pendingDef = std::move(m_lookaheadDef);
    m_lookaheadDef = GenerateWave(data.m_waveNumber + 2);
    m_pendingBudget = m_lookaheadBudget;
    m_lookaheadBudget = BudgetForWave(data.m_waveNumber + 2);

    // Rebuild the preview pool for the new upcoming wave (next wave's tier) for the HUD.
    RebuildPreviewPrototypes(data.m_waveNumber + 1, enemyFactory);
}
