#include <systems/tower_system.hpp>

#include <raymath.h>
#include <algorithm>

#include <world/tower.hpp>
#include <world/enemy_modules.hpp>



void TowerSystem::Update(float dt, GameData& gameData, ParticleSystem& particles){
    for (Tower& tower : gameData.towers) {
        // Recompute live stats from base, then let modules override/augment
        tower.m_stats = tower.m_base;
        for (auto& mod : tower.m_modules)
            mod->ContributeTower(tower.m_stats);

        if (tower.m_role == TowerRole::Wall) continue;

        tower.m_cooldown -= dt;
        // Decay the attack flash over the visual's attackDuration; guard against a zero duration
        if (tower.m_visual.attackDuration > 0.0f)
            tower.m_attackFlashRatio = std::max(0.0f, tower.m_attackFlashRatio - dt / tower.m_visual.attackDuration);
        else
            tower.m_attackFlashRatio = 0.0f;

        if (tower.m_cooldown > 0.0f) continue;

        std::vector<DenseSlotMap<Enemy>::Key> targetKeys = FindTargets(tower, gameData.enemies, tower.m_stats.targetCount);

        if (targetKeys.empty()) {
            tower.m_cooldown = 0.05f;
            continue;
        }

        tower.m_cooldown         = (tower.m_stats.shotsPerMinute > 0.0f) ? 60.0f / tower.m_stats.shotsPerMinute : 999.0f;
        tower.m_attackFlashRatio = 1.0f;

        std::vector<Vector2> targetPositions;
        targetPositions.reserve(targetKeys.size());
        for (auto& key : targetKeys) {
            if (Enemy* e = gameData.enemies.Get(key))
                targetPositions.push_back(e->m_position);
        }

        // Damage payload
        AttackPayload payload;
        payload.m_targetKeys = targetKeys;
        BuildPayload(tower, payload);

        // Visual effect
        VfxEffect vfx;
        vfx.m_origin = tower.m_position;
        vfx.m_targetPositions = std::move(targetPositions);
        vfx.m_duration = tower.m_visual.attackDuration;
        vfx.m_maxDuration = tower.m_visual.attackDuration;
        vfx.m_radius = tower.m_stats.range;
        vfx.m_color = tower.m_visual.color;
        vfx.m_style = tower.m_visual.style;
        // Muzzle burst
        if (tower.m_visual.muzzleDesc.count > 0)
            particles.Emit(tower.m_position, tower.m_visual.muzzleDesc);

        gameData.m_payloads.push_back(std::move(payload));
        gameData.m_vfx.push_back(std::move(vfx));
    }
}

std::vector<DenseSlotMap<Enemy>::Key> TowerSystem::FindTargets(Tower& tower, DenseSlotMap<Enemy>& enemies, int max_targets) {
    std::vector<Enemy*> inRange = FindEnemiesInRange(tower, enemies);

    std::sort(inRange.begin(), inRange.end(), [&](const Enemy* a, const Enemy* b) {
        return CompareTarget(*a, *b, tower.m_stats.targetingMode);
    });

    std::vector<DenseSlotMap<Enemy>::Key> result;
    int count = std::min(static_cast<int>(inRange.size()), max_targets);

    // count == 0 means target all enemies in range
    if (count == 0)
        count = static_cast<int>(inRange.size());

    result.reserve(count);
    for (int i = 0; i < count; i++)
        result.push_back(enemies.KeyOf(inRange[i]));

    return result;
}

std::vector<Enemy*> TowerSystem::FindEnemiesInRange(Tower& tower, DenseSlotMap<Enemy>& enemies) {
    std::vector<Enemy*> result;
    for (auto& enemy : enemies) {
        if (enemy.m_currentHealth <= 0.0f) continue;
        if (tower.m_stats.range >= Vector2Distance(enemy.m_position, tower.m_position))
            result.push_back(&enemy);
    }
    return result;
}

void TowerSystem::BuildPayload(const Tower& tower, AttackPayload& payload) {
    // Scalar combat values come from stats; behavioural effects come from modules
    payload.m_damage = tower.m_stats.damage;
    payload.m_armorPierce = tower.m_stats.armorPierce;
    payload.m_critChance = tower.m_stats.critChance;
    payload.m_critMultiplier = tower.m_stats.critMultiplier;
    if (tower.m_visual.impactDesc.count > 0)
        payload.m_impactDescs.push_back(tower.m_visual.impactDesc);
    if (tower.m_visual.critImpactDesc.count > 0)
        payload.m_critImpactDescs.push_back(tower.m_visual.critImpactDesc);
    for (auto& mod : tower.m_modules)
        mod->Contribute(payload);
}

void TowerSystem::TickPayloads(GameData& gameData, ParticleSystem& particles) {
    for (auto& payload : gameData.m_payloads) {
        if (payload.m_resolved) continue;

        for (auto& key : payload.m_targetKeys) {
            Enemy* enemy = gameData.enemies.Get(key);
            if (!enemy) continue;

            float armor = std::max(0.0f, enemy->m_stats.armor - payload.m_armorPierce);
            float dmg = payload.m_damage;
            bool crit = payload.m_critChance > 0.0f
                && GetRandomValue(0, 99) < (int)(payload.m_critChance * 100.0f);
            if (crit)
                dmg *= payload.m_critMultiplier;
            float net = std::max(0.0f, dmg - armor);
            for (auto& mod : enemy->m_modules)
                net = mod->InterceptDamage(net);
            enemy->m_currentHealth -= net;
            for (auto& effect : payload.m_effects)
                enemy->AddEffect(effect);

            // Impact particles — all modules contribute their burst; crit adds extra descs
            for (auto& desc : payload.m_impactDescs)
                particles.Emit(enemy->m_position, desc);
            if (crit) {
                for (auto& desc : payload.m_critImpactDescs)
                    particles.Emit(enemy->m_position, desc);
            }
        }
        payload.m_resolved = true;
    }
    std::erase_if(gameData.m_payloads, [](const AttackPayload& p) { return p.m_resolved; });
}

void TowerSystem::TickVfx(float dt, GameData& gameData) {
    for (auto& vfx : gameData.m_vfx)
        vfx.m_duration -= dt;
    std::erase_if(gameData.m_vfx, [](const VfxEffect& v) { return v.m_duration <= 0.0f; });
}

static float TotalShield(const Enemy& enemy) {
    float total = 0.0f;
    for (auto& mod : enemy.m_modules)
        total += mod->GetShield();
    return total;
}

bool TowerSystem::CompareTarget(const Enemy& a, const Enemy& b, TargetingMode mode) {
    switch (mode) {
        case TargetingMode::First:          return a.m_progress < b.m_progress;
        case TargetingMode::Last:           return a.m_progress > b.m_progress;
        case TargetingMode::MostHealth:     return a.m_currentHealth > b.m_currentHealth;
        case TargetingMode::LowestHealth:   return a.m_currentHealth < b.m_currentHealth;
        case TargetingMode::Fastest:        return a.m_stats.speed < b.m_stats.speed;
        case TargetingMode::Slowest:        return a.m_stats.speed > b.m_stats.speed;
        case TargetingMode::MostArmor:      return a.m_stats.armor > b.m_stats.armor;
        case TargetingMode::MostResistance: return a.m_stats.resistance > b.m_stats.resistance;
        case TargetingMode::MostShield:     return TotalShield(a) > TotalShield(b);
    }
    return false;
}
