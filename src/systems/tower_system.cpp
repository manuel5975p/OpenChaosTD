#include <systems/tower_system.hpp>

#include <raymath.h>
#include <algorithm>

#include <world/tower.hpp>
#include <world/tower_modules.hpp>
#include <world/effect.hpp>


void TowerSystem::update(float& dt, GameData& gameData){
    for (Tower& tower : gameData.towers) {
        tower.m_cooldown -= dt;
        tower.m_attackFlash = std::max(0.0f, tower.m_attackFlash - dt);

        tower.m_currentTargetKeys = FindTargets(tower, gameData.enemies, tower.m_targetCount);

        if(tower.m_cooldown <= 0 && !tower.m_currentTargetKeys.empty()){
            tower.m_cooldown = 1.0f / tower.m_fireRate;
            tower.m_attackFlash = 0.15f;

            std::vector<Vector2> targetPositions;
            targetPositions.reserve(tower.m_currentTargetKeys.size());
            for (auto& key : tower.m_currentTargetKeys) {
                if (Enemy* e = gameData.enemies.Get(key))
                    targetPositions.push_back(e->m_position);
            }

            constexpr float kAttackDuration = 0.15f;
            Attack attack;
            attack.m_origin          = tower.m_position;
            attack.m_targetPositions = std::move(targetPositions);
            attack.m_targetKeys      = tower.m_currentTargetKeys;
            attack.m_type            = tower.m_attackType;
            attack.m_radius          = tower.m_radius;
            attack.m_duration        = kAttackDuration;
            attack.m_maxDuration     = kAttackDuration;
            BuildAttackPayload(tower, attack);
            gameData.attacks.push_back(std::move(attack));
        }
    }
}

std::vector<DenseSlotMap<Enemy>::Key> TowerSystem::FindTargets(Tower& tower, DenseSlotMap<Enemy>& enemies, int max_targets) {
    std::vector<Enemy*> inRange = FindEnemiesInRange(tower, enemies);

    // Sort by targeting priority
    std::sort(inRange.begin(), inRange.end(), [&](const Enemy* a, const Enemy* b) {
        return CompareTarget(*a, *b, tower.m_targetingMode);
    });

    // Take up to max_targets and convert to stable keys
    std::vector<DenseSlotMap<Enemy>::Key> result;
    int count = std::min(static_cast<int>(inRange.size()), max_targets);
    
    // Set count to size of inRange to return all enemys in range
    if(count == 0){
        count = static_cast<int>(inRange.size());
    }

    result.reserve(count);
    for (int i = 0; i < count; i++)
        result.push_back(enemies.KeyOf(inRange[i]));

    return result;
}

std::vector<Enemy*> TowerSystem::FindEnemiesInRange(Tower& tower, DenseSlotMap<Enemy>& enemies) {
    std::vector<Enemy*> result;
    for (auto& enemy : enemies) {
        if (tower.m_radius >= Vector2Distance(enemy.m_position, tower.m_position))
            result.push_back(&enemy);
    }
    return result;
}

void TowerSystem::BuildAttackPayload(const Tower& tower, Attack& attack) {
    for (auto& mod : tower.m_modules) {
        if (auto* dmg = dynamic_cast<FlatDamageModule*>(mod.get())) {
            attack.m_damage += dmg->m_damage;
        } else if (auto* slow = dynamic_cast<SlowModule*>(mod.get())) {
            attack.m_effects.push_back(Effect(EffectType::Slow, slow->m_duration, slow->m_factor));
        }
    }
}

bool TowerSystem::CompareTarget(const Enemy& a, const Enemy& b, TargetingMode mode) {
    switch (mode) {
        case TargetingMode::First: return a.m_progress < b.m_progress;
        case TargetingMode::Last: return a.m_progress > b.m_progress;
        case TargetingMode::MostHealth: return a.m_currentHealth < b.m_currentHealth;
        case TargetingMode::LowestHealth: return a.m_currentHealth > b.m_currentHealth;
        case TargetingMode::Fastest: return a.m_speed < b.m_speed;
        case TargetingMode::Slowest: return a.m_speed > b.m_speed;
    }
    return false;
}