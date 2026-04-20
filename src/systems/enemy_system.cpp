#include <systems/enemy_system.hpp>

#include <raymath.h>

void EnemySystem::UpdateEnemyPosition(float& dt, GameData& gameData){
    for (auto& enemy : gameData.enemies) {
        float remainingTime = dt;

        // Enemy will not move this frame
        if(enemy.m_speed <= 0.0f) continue;
 
        // Loop to handle enemies fast enough to cross multiple waypoints in one frame
        while(remainingTime > 0.0f && enemy.m_waypointIndex >= 0){
 
            Vector2 toTarget = Vector2Subtract(gameData.map.GetPaths()[enemy.m_spawnedNest][enemy.m_waypointIndex], enemy.m_position);
            float distToTarget = Vector2Length(toTarget);
            float moveDistance = enemy.m_speed * remainingTime;
 
            if(moveDistance >= distToTarget){
                // Snap to waypoint and carry over the unused time
                remainingTime -= distToTarget / enemy.m_speed;
                enemy.m_position = gameData.map.GetPaths()[enemy.m_spawnedNest][enemy.m_waypointIndex];
                enemy.m_waypointIndex--; // Reaches -1 when core is hit
            } else {
                // Move as far as we can this frame
                Vector2 direction = Vector2Normalize(toTarget);
                enemy.m_position = Vector2Add(enemy.m_position, Vector2Scale(direction, moveDistance));
                remainingTime = 0.0f;

                // Update progress to next waypint
                enemy.m_progress = enemy.m_waypointIndex + distToTarget / gameData.map.GetTileSize();
            }
        }
    }
}