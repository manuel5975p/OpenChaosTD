#include <systems/world_system.hpp>
#include <world/tile.hpp>
#include <iostream>
#include <raymath.h>

void WorldSystem::PlaceTower(int x, int y, Tower& towerTemplate, GameData& gameData){
    if(ValidateTowerPlacement(x, y, gameData)){
        Tile& tile = gameData.map.Get(x, y);

        // Add tower
        towerTemplate.m_position = gameData.map.TileToWorld(x, y);
        DenseSlotMap<Tower>::Key towerKey = gameData.towers.Insert(std::move(towerTemplate));
        
        tile.m_walkable = false;
        tile.m_buildable = false;
        tile.m_towerKey = towerKey;

        std::cout << "Tower placed x: " << x << " y: " << y << std::endl;
    }
}

void WorldSystem::RemoveTower(int x, int y, GameData& gameData){
    Tile& tile = gameData.map.Get(x, y);

    if(tile.m_towerKey != DenseSlotMap<Tower>::INVALID_KEY){
        // Remove tower
        gameData.towers.Erase(tile.m_towerKey);

        tile.m_walkable = true;
        tile.m_buildable = true;
        tile.m_towerKey = DenseSlotMap<Tower>::INVALID_KEY;
        std::cout << "Tower removed x: " << x << " y: " << y << std::endl;

        gameData.map.BuildPathMesh();
    }
}

void WorldSystem::SpawnEnemy(const int& nest, const Enemy& templateEnemy,GameData& gameData){
    Enemy newEnemy;
    newEnemy.m_speed = templateEnemy.m_speed;
    newEnemy.m_name = templateEnemy.m_name;
    newEnemy.m_maxhealth = templateEnemy.m_maxhealth;
    newEnemy.m_health = templateEnemy.m_health;

    newEnemy.m_position = {
        static_cast<float>(gameData.map.GetNests()[nest].first * gameData.map.GetTileSize() + static_cast<float>(gameData.map.GetTileSize()) /2), 
        static_cast<float>(gameData.map.GetNests()[nest].second * gameData.map.GetTileSize()+ static_cast<float>(gameData.map.GetTileSize()) /2)
    };

    newEnemy.m_spawnedNest = nest;
    newEnemy.m_waypointIndex = gameData.map.GetPaths()[nest].size() -2;

    gameData.enemies.Insert(newEnemy);
}

void WorldSystem::RemoveEnemy(){
    
}

bool WorldSystem::ValidateTowerPlacement(int x, int y, GameData& gameData){
    Tile& tile = gameData.map.Get(x, y);

    // Return if tile not buildable
    if(!tile.m_buildable){
        std::cout << "Tower not placed x: " << x << " y: " << y << " tile is not buildable" << std::endl;
        return false;
    }

    // Check if paths are still valid after tower placement
    tile.m_walkable = false;
    gameData.map.BuildPathMesh();
    if(!gameData.map.ValidatePathMesh()){
        tile.m_walkable = true;
        gameData.map.BuildPathMesh();
        std::cout << "Tower not placed x: " << x << " y: " << y << " blocks path to core" << std::endl;
        return false;
    }

    // If nothing fails allow tower placement
    return true;
}

void WorldSystem::UpdateEnemyPosition(float& dt, GameData& gameData){
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

void WorldSystem::GenerateMap(Map& map, int x, int y){
    map.Create(x, y);

    int xmid = (map.GetCols() -1) /2;
    int ymid = (map.GetRows() -1) /2;
    for(int x=0; x < map.GetCols(); x++) {
        for(int y=0; y < map.GetRows(); y++) {
            
            //Add Nest at the top
            if(y == 1 && x == xmid) map.AddNest(x, y);
            if(y == 1 && x == 1) map.AddNest(x, y);
            if(y == 1 && x == map.GetCols()-2) map.AddNest(x, y);

            //Place Core at the bottom
            if(y == map.GetRows() -2 && x == xmid) map.SetCore(x, y);

            // Add Rock formation
            Tile rockTile;
            rockTile.m_type = TileType::Rock;
            rockTile.m_buildable = false;
            rockTile.m_walkable = false;

            if(y == ymid && x < map.GetCols() -3) map.Get(x, y) = std::move(rockTile);

            
        }
    }
}