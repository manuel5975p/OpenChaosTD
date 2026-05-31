#include <systems/tower_system.hpp>

#include <raymath.h>
#include <algorithm>
#include <cmath>

#include <world/tower.hpp>
#include <world/enemy_modules.hpp>

static constexpr float TrailEmitRate = 25.0f;


void TowerSystem::update(float dt, GameData& gameData, ParticleSystem& particles){
    for (Tower& tower : gameData.towers) {
        // Recompute live stats from base, then let modules override/augment
        tower.m_stats = tower.m_base;
        for (auto& mod : tower.m_modules)
            mod->ContributeTower(tower.m_stats);

        if (tower.m_role == TowerRole::Wall) continue;

        tower.m_cooldown -= dt;
        tower.m_attackFlashRatio = std::max(0.0f, tower.m_attackFlashRatio - dt / tower.m_stats.attackDuration);

        if (tower.m_cooldown > 0.0f) continue;

        std::vector<DenseSlotMap<Enemy>::Key> targetKeys = FindTargets(tower, gameData.enemies, tower.m_stats.targetCount);

        if (targetKeys.empty()) {
            tower.m_cooldown = 0.05f;
            continue;
        }

        tower.m_cooldown         = 1.0f / tower.m_stats.fireRate;
        tower.m_attackFlashRatio = 1.0f;

        std::vector<Vector2> targetPositions;
        targetPositions.reserve(targetKeys.size());
        for (auto& key : targetKeys) {
            if (Enemy* e = gameData.enemies.Get(key))
                targetPositions.push_back(e->m_position);
        }

        // Damage payload
        AttackPayload payload;
        payload.m_ttl = tower.m_stats.attackDuration;
        payload.m_targetKeys = targetKeys;
        BuildPayload(tower, payload);

        // Visual effect
        VfxEffect vfx;
        vfx.m_origin = tower.m_position;
        vfx.m_targetPositions = std::move(targetPositions);
        vfx.m_duration = tower.m_stats.attackDuration;
        vfx.m_maxDuration = tower.m_stats.attackDuration;
        vfx.m_radius = tower.m_stats.radius;
        vfx.m_color = tower.m_vfx.color;
        vfx.m_style = tower.m_vfx.style;
        vfx.m_trailDesc = payload.m_trailDesc;

        // Muzzle burst
        if (tower.m_vfx.muzzleDesc.count > 0)
            particles.Emit(tower.m_position, tower.m_vfx.muzzleDesc);

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
        if (tower.m_stats.radius >= Vector2Distance(enemy.m_position, tower.m_position))
            result.push_back(&enemy);
    }
    return result;
}

void TowerSystem::BuildPayload(const Tower& tower, AttackPayload& payload) {
    for (auto& mod : tower.m_modules)
        mod->Contribute(payload);
}

void TowerSystem::TickPayloads(float dt, GameData& gameData, ParticleSystem& particles) {
    for (auto& payload : gameData.m_payloads) {
        payload.m_ttl -= dt;

        if (payload.m_resolved) continue;
        payload.m_delay -= dt;
        if (payload.m_delay > 0.0f) continue;

        for (auto& key : payload.m_targetKeys) {
            Enemy* enemy = gameData.enemies.Get(key);
            if (!enemy) continue;

            float armor = std::max(0.0f, enemy->m_stats.armor - payload.m_armorPierce);
            float dmg = payload.m_damage;
            bool crit = payload.m_critChance > 0.0f
                && GetRandomValue(0, 99) < (int)(payload.m_critChance * 100.0f);
            if (crit) {
                dmg *= payload.m_critMultiplier;
                payload.m_wasCrit = true;
            }
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
    std::erase_if(gameData.m_payloads, [](const AttackPayload& p) { return p.m_ttl <= 0.0f; });
}

// Returns a world-space point suitable for spawning a trail particle from this effect
static Vector2 SampleVfxPoint(const VfxEffect& vfx) {
    if (vfx.m_style == VfxStyle::Ring) {
        float angle = (float)GetRandomValue(0, 62831) / 10000.0f;
        float r = (1.0f - vfx.Progress()) * vfx.m_radius;
        return {vfx.m_origin.x + std::cosf(angle) * r,
                vfx.m_origin.y + std::sinf(angle) * r};
    }
    if (!vfx.m_targetPositions.empty()) {
        int idx = GetRandomValue(0, (int)vfx.m_targetPositions.size() - 1);
        float t = (float)GetRandomValue(0, 10000) / 10000.0f;
        return Vector2Lerp(vfx.m_origin, vfx.m_targetPositions[idx], t);
    }
    return vfx.m_origin;
}

void TowerSystem::TickVfx(float dt, GameData& gameData, ParticleSystem& particles) {
    for (auto& vfx : gameData.m_vfx) {
        vfx.m_duration -= dt;

        if (vfx.m_trailDesc.count > 0) {
            vfx.m_trailAccumulator += TrailEmitRate * dt;
            while (vfx.m_trailAccumulator >= 1.0f) {
                particles.Emit(SampleVfxPoint(vfx), vfx.m_trailDesc);
                vfx.m_trailAccumulator -= 1.0f;
            }
        }
    }
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
