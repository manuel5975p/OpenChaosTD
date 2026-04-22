#include <systems/world_system.hpp>
#include <world/tile.hpp>
#include <iostream>
#include <raymath.h>

void WorldSystem::PlaceTower(int x, int y, Tower& tower, GameData& gameData){
    if(ValidateTowerPlacement(x, y, gameData)){
        Tile& tile = gameData.map.Get(x, y);

        // Add tower
        tower.m_position = Vector2Add(gameData.map.TileToWorld(x, y), {gameData.map.GetTileSize() /2.f, gameData.map.GetTileSize() /2.f});
        DenseSlotMap<Tower>::Key towerKey = gameData.towers.Insert(std::move(tower));
        
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

void WorldSystem::SpawnEnemy(const int& nest, Enemy enemy,GameData& gameData){
    enemy.m_position = {
        static_cast<float>(gameData.map.GetNests()[nest].first * gameData.map.GetTileSize() + static_cast<float>(gameData.map.GetTileSize()) /2), 
        static_cast<float>(gameData.map.GetNests()[nest].second * gameData.map.GetTileSize()+ static_cast<float>(gameData.map.GetTileSize()) /2)
    };

    enemy.m_spawnedNest = nest;
    enemy.m_waypointIndex = gameData.map.GetPaths()[nest].size() -2;

    gameData.enemies.Insert(std::move(enemy));
}

void WorldSystem::RemoveEnemy(DenseSlotMap<Enemy>::Key key, GameData& gameData){
    gameData.enemies.Erase(key);
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

void WorldSystem::CheckEnemyReachedCore(GameData& gameData){
    std::vector<DenseSlotMap<Enemy>::Key> enemyErase;
    for (auto& enemy: gameData.enemies) {
        // Enemy reached core
        if(enemy.m_waypointIndex == -1){
            enemyErase.push_back(gameData.enemies.KeyOf(&enemy));
        }   
    }

    for(auto& erase : enemyErase){
        gameData.lives --;
        RemoveEnemy(erase, gameData);
    }
}

void WorldSystem::CheckGameOver(bool& gameOver, GameData& gameData){
    // Core live reaches zero
    if(gameData.lives <= 0){
        gameOver = true;
    }
}
