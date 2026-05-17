#include <systems/enemy_system.hpp>

#include <raymath.h>

void EnemySystem::FollowPath(float dt, GameData& gameData){
    for (auto& enemy : gameData.enemies) {
        if(enemy.m_currentSpeed <= 0.0f) continue;

        float remainingTime = dt;

        while(remainingTime > 0.0f && enemy.m_waypointIndex >= 0){
            Vector2 toTarget = Vector2Subtract(gameData.map.GetPaths()[enemy.m_spawnedNest][enemy.m_waypointIndex], enemy.m_position);
            float distToTarget = Vector2Length(toTarget);
            float moveDistance = enemy.m_currentSpeed * remainingTime;

            if(moveDistance >= distToTarget){
                remainingTime -= distToTarget / enemy.m_currentSpeed;
                enemy.m_position = gameData.map.GetPaths()[enemy.m_spawnedNest][enemy.m_waypointIndex];
                enemy.m_waypointIndex--;
                enemy.m_progress = static_cast<float>(enemy.m_waypointIndex + 1);
            } else {
                Vector2 direction = Vector2Normalize(toTarget);
                enemy.m_position = Vector2Add(enemy.m_position, Vector2Scale(direction, moveDistance));
                remainingTime = 0.0f;
                float remainingDist = distToTarget - moveDistance;
                enemy.m_progress = enemy.m_waypointIndex + remainingDist / gameData.map.GetTileSize();
            }
        }
    }
}

void EnemySystem::TickEnemies(float dt, GameData& gameData){
    for (auto& enemy : gameData.enemies) {
        enemy.m_currentSpeed = enemy.m_speed;
        enemy.m_resistance = 0.0f;

        for (auto& mod : enemy.m_modules)
            mod->Tick(dt, enemy);

        for (auto& effect : enemy.m_effects) {
            switch (effect.m_type) {
                case EffectType::Burn:
                    enemy.m_currentHealth -= effect.m_value * (1.0f - enemy.m_resistance) * dt;
                    break;
                case EffectType::Slow: {
                    float weakenedFactor = effect.m_value + (1.0f - effect.m_value) * enemy.m_resistance;
                    enemy.m_currentSpeed *= weakenedFactor;
                    break;
                }
            }
            effect.m_duration -= dt;
        }
        std::erase_if(enemy.m_effects, [](const Effect& e){ return e.m_duration <= 0.0f; });
    }
}