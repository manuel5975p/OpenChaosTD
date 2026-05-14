#include <systems/enemy_system.hpp>

#include <raymath.h>

void EnemySystem::FollowPath(float& dt, GameData& gameData){
    for (auto& enemy : gameData.enemies) {
        float effectiveSpeed = enemy.m_speed;
        for (auto& effect : enemy.m_effects) {
            if (effect.m_type == EffectType::Slow)
                effectiveSpeed *= effect.m_value;
        }

        if(effectiveSpeed <= 0.0f) continue;

        float remainingTime = dt;

        // Loop to handle enemies fast enough to cross multiple waypoints in one frame
        while(remainingTime > 0.0f && enemy.m_waypointIndex >= 0){

            Vector2 toTarget = Vector2Subtract(gameData.map.GetPaths()[enemy.m_spawnedNest][enemy.m_waypointIndex], enemy.m_position);
            float distToTarget = Vector2Length(toTarget);
            float moveDistance = effectiveSpeed * remainingTime;

            if(moveDistance >= distToTarget){
                // Snap to waypoint and carry over the unused time
                remainingTime -= distToTarget / effectiveSpeed;
                enemy.m_position = gameData.map.GetPaths()[enemy.m_spawnedNest][enemy.m_waypointIndex];
                enemy.m_waypointIndex--; // Reaches -1 when core is hit
            } else {
                // Move as far as we can this frame
                Vector2 direction = Vector2Normalize(toTarget);
                enemy.m_position = Vector2Add(enemy.m_position, Vector2Scale(direction, moveDistance));
                remainingTime = 0.0f;

                // Update progress to next waypoint
                enemy.m_progress = enemy.m_waypointIndex + distToTarget / gameData.map.GetTileSize();
            }
        }
    }
}

void EnemySystem::TickEffects(float& dt, GameData& gameData){
    for (auto& enemy : gameData.enemies) {
        for (auto& effect : enemy.m_effects){
            switch (effect.m_type) {
                case EffectType::Burn:
                    enemy.m_health -= effect.m_value * dt;
                    break;
                case EffectType::Slow:
                    break;
            }
            effect.m_duration -= dt;
        }
        std::erase_if(enemy.m_effects, [](const Effect& e){ return e.m_duration <= 0.0f; });
    }
}