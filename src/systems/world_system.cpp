#include <systems/world_system.hpp>
#include <world/tile.hpp>
#include <world/enemy_module.hpp>
#include <factory/enemy_factory.hpp>
#include <iostream>
#include <raymath.h>
#include <vector>

bool WorldSystem::PlaceTower(int x, int y, Tower& tower, GameData& gameData){
    if(!ValidateTowerPlacement(x, y, gameData)) return false;

    Tile& tile = gameData.map.Get(x, y);
    tower.m_position = Vector2Add(gameData.map.TileToWorld(x, y), {gameData.map.GetTileSize() /2.f, gameData.map.GetTileSize() /2.f});
    DenseSlotMap<Tower>::Key towerKey = gameData.towers.Insert(std::move(tower));

    tile.m_walkable = false;
    tile.m_buildable = false;
    tile.m_towerKey = towerKey;
    return true;
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

void WorldSystem::SpawnEnemy(int nest, Enemy&& enemy, GameData& gameData){
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
    for(int col=0; col < map.GetCols(); col++) {
        for(int row=0; row < map.GetRows(); row++) {

            if(row == 1 && col == xmid) map.AddNest(col, row);
            if(row == 1 && col == 1) map.AddNest(col, row);
            if(row == 1 && col == map.GetCols()-2) map.AddNest(col, row);

            if(row == map.GetRows() -2 && col == xmid) map.SetCore(col, row);

            Tile rockTile;
            rockTile.m_type = TileType::Rock;
            rockTile.m_buildable = false;
            rockTile.m_walkable = false;

            if(row == ymid && col < map.GetCols() -3) map.Get(col, row) = std::move(rockTile);
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
        gameData.lives -= gameData.enemies.Get(erase)->m_livesOnReach;
        RemoveEnemy(erase, gameData);
    }
}

void WorldSystem::CheckEnemyDead(GameData& gameData, EnemyFactory& enemyFactory){
    std::vector<DenseSlotMap<Enemy>::Key> toRemove;
    for (auto& enemy : gameData.enemies) {
        if (enemy.m_currentHealth <= 0.0f)
            toRemove.push_back(gameData.enemies.KeyOf(&enemy));
    }
    for (auto& key : toRemove) {
        Enemy* enemy = gameData.enemies.Get(key);
        gameData.gold += enemy->m_reward;
        SpawnSplitChildren(*enemy, gameData, enemyFactory);
        RemoveEnemy(key, gameData);
    }
}

void WorldSystem::SpawnSplitChildren(const Enemy& parent, GameData& gameData, EnemyFactory& enemyFactory){
    for (const auto& mod : parent.m_modules) {
        auto* split = dynamic_cast<const SplitModule*>(mod.get());
        if (!split || !enemyFactory.Has(split->m_childType)) continue;

        for (int i = 0; i < split->m_count; i++) {
            Enemy child = enemyFactory.Create(split->m_childType);
            // Children resume the parent's path from where it died
            child.m_position = parent.m_position;
            child.m_spawnedNest = parent.m_spawnedNest;
            child.m_waypointIndex = parent.m_waypointIndex;
            child.m_progress = parent.m_progress;
            gameData.enemies.Insert(std::move(child));
        }
    }
}


void WorldSystem::CheckGameOver(bool& gameOver, GameData& gameData){
    // Core live reaches zero
    if(gameData.lives <= 0){
        gameOver = true;
    }
}
