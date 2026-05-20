#include <systems/enemy_system.hpp>

#include <raymath.h>

void EnemySystem::FollowPath(float dt, GameData& gameData){
    for (auto& enemy : gameData.enemies) {
        if (enemy.m_stats.speed <= 0.0f) continue;

        float remainingTime = dt;

        while (remainingTime > 0.0f && enemy.m_waypointIndex >= 0) {
            Vector2 toTarget = Vector2Subtract(gameData.map.GetPaths()[enemy.m_spawnedNest][enemy.m_waypointIndex], enemy.m_position);
            float distToTarget = Vector2Length(toTarget);
            float moveDistance = enemy.m_stats.speed * remainingTime;

            if (moveDistance >= distToTarget) {
                remainingTime -= distToTarget / enemy.m_stats.speed;
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
        // Recompute live stats from base, then let modules contribute
        enemy.m_stats = enemy.m_base;
        for (auto& mod : enemy.m_modules)
            mod->ContributeStats(enemy.m_stats);

        for (auto& mod : enemy.m_modules)
            mod->Tick(dt, enemy);

        // Compute enemy velocity so status-effect particles inherit it and don't lag behind
        Vector2 baseVel = {0, 0};
        if (enemy.m_waypointIndex >= 0) {
            Vector2 toWaypoint = Vector2Subtract(
                gameData.map.GetPaths()[enemy.m_spawnedNest][enemy.m_waypointIndex],
                enemy.m_position
            );
            float dist = Vector2Length(toWaypoint);
            if (dist > 0.001f)
                baseVel = Vector2Scale(Vector2Normalize(toWaypoint), enemy.m_stats.speed);
        }

        for (auto& effect : enemy.m_effects) {
            // Emit visual particles for active effects (burn flicker, slow drift)
            if (effect.m_emitRate > 0.0f) {
                effect.m_emitAccumulator += effect.m_emitRate * dt;
                while (effect.m_emitAccumulator >= 1.0f) {
                    gameData.particles.Emit(enemy.m_position, effect.m_particleDesc, baseVel);
                    effect.m_emitAccumulator -= 1.0f;
                }
            }

            switch (effect.m_type) {
                case EffectType::Burn:
                    enemy.m_currentHealth -= effect.m_value * (1.0f - enemy.m_stats.resistance) * dt;
                    break;
                case EffectType::Slow: {
                    float weakenedFactor = effect.m_value + (1.0f - effect.m_value) * enemy.m_stats.resistance;
                    enemy.m_stats.speed *= weakenedFactor;
                    break;
                }
            }
            effect.m_duration -= dt;
        }
        std::erase_if(enemy.m_effects, [](const Effect& e){ return e.m_duration <= 0.0f; });
    }
}
