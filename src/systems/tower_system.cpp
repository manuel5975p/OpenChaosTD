#include <systems/tower_system.hpp>

#include <raymath.h>

#include <world/tower.hpp>


void TowerSystem::update(float& dt, GameData& gameData){
    for (Tower& tower : gameData.towers) {
        tower.m_cooldown -= dt;

        // Attack
        if(tower.m_cooldown <= 0){
            std::cout << "Attack" << std::endl;
            tower.m_cooldown = 1.0f / tower.m_fireRate; // Reset
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

bool TowerSystem::CompareTarget(const Enemy& a, const Enemy& b, TargetingMode mode) {
    switch (mode) {
        case TargetingMode::First: return a.m_progress < b.m_progress;
        case TargetingMode::Last: return a.m_progress > b.m_progress;
        case TargetingMode::MostHealth: return a.m_health < b.m_health;
        case TargetingMode::LowestHealth: return a.m_health > b.m_health;
        case TargetingMode::Fastest: return a.m_speed < b.m_speed;
        case TargetingMode::Slowest: return a.m_speed > b.m_speed;
    }
    return false;
}