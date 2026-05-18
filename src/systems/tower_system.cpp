#include <systems/tower_system.hpp>

#include <raymath.h>
#include <algorithm>

#include <world/tower.hpp>
#include <world/enemy_modules.hpp>


void TowerSystem::update(float dt, GameData& gameData){
    for (Tower& tower : gameData.towers) {
        tower.m_cooldown -= dt;
        tower.m_attackFlash = std::max(0.0f, tower.m_attackFlash - dt);

        if (tower.m_cooldown > 0.0f) continue;

        tower.m_currentTargetKeys = FindTargets(tower, gameData.enemies, tower.m_targetCount);

        if (tower.m_currentTargetKeys.empty()) {
            tower.m_cooldown = 0.05f;
            continue;
        }

        tower.m_cooldown = 1.0f / tower.m_fireRate;
        tower.m_attackFlash = tower.m_attackDuration;

        std::vector<Vector2> targetPositions;
        targetPositions.reserve(tower.m_currentTargetKeys.size());
        for (auto& key : tower.m_currentTargetKeys) {
            if (Enemy* e = gameData.enemies.Get(key))
                targetPositions.push_back(e->m_position);
        }

        Attack attack;
        attack.m_origin = tower.m_position;
        attack.m_targetPositions = std::move(targetPositions);
        attack.m_targetKeys = tower.m_currentTargetKeys;
        attack.m_type = tower.m_attackType;
        attack.m_radius = tower.m_radius;
        attack.m_duration = tower.m_attackDuration;
        attack.m_maxDuration = tower.m_attackDuration;
        BuildAttackPayload(tower, attack);
        gameData.attacks.push_back(std::move(attack));
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
        if (enemy.m_currentHealth <= 0.0f) continue;
        if (tower.m_radius >= Vector2Distance(enemy.m_position, tower.m_position))
            result.push_back(&enemy);
    }
    return result;
}

void TowerSystem::BuildAttackPayload(const Tower& tower, Attack& attack) {
    for (auto& mod : tower.m_modules)
        mod->Contribute(attack);
}

static float ComputeArmor(const Enemy& enemy) {
    float armor = 0.0f;
    for (auto& mod : enemy.m_modules)
        armor += mod->GetArmor();
    return armor;
}

void TowerSystem::TickAttacks(float dt, GameData& gameData) {
    for (auto& attack : gameData.attacks) {
        attack.m_duration -= dt;

        if (attack.m_resolved) continue;
        attack.m_delay -= dt;
        if (attack.m_delay > 0.0f) continue;

        for (auto& key : attack.m_targetKeys) {
            Enemy* enemy = gameData.enemies.Get(key);
            if (!enemy) continue;
            float armor = std::max(0.0f, ComputeArmor(*enemy) - attack.m_armorPierce);
            float dmg = attack.m_damage;
            if (attack.m_critChance > 0.0f && GetRandomValue(0, 99) < (int)(attack.m_critChance * 100.0f))
                dmg *= attack.m_critMultiplier;
            enemy->m_currentHealth -= std::max(0.0f, dmg - armor);
            for (auto& effect : attack.m_effects)
                enemy->AddEffect(effect);
        }
        attack.m_resolved = true;
    }
    std::erase_if(gameData.attacks, [](const Attack& a){ return a.m_duration <= 0.0f; });
}

bool TowerSystem::CompareTarget(const Enemy& a, const Enemy& b, TargetingMode mode) {
    switch (mode) {
        case TargetingMode::First: return a.m_progress < b.m_progress;
        case TargetingMode::Last: return a.m_progress > b.m_progress;
        case TargetingMode::MostHealth: return a.m_currentHealth > b.m_currentHealth;
        case TargetingMode::LowestHealth: return a.m_currentHealth < b.m_currentHealth;
        case TargetingMode::Fastest: return a.m_speed < b.m_speed;
        case TargetingMode::Slowest: return a.m_speed > b.m_speed;
    }
    return false;
}