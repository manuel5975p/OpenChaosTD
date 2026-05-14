#include <systems/world_system.hpp>
#include <world/tile.hpp>
#include <world/enemy_module.hpp>
#include <iostream>
#include <raymath.h>
#include <vector>
#include <algorithm>

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
        gameData.lives --;
        RemoveEnemy(erase, gameData);
    }
}

void WorldSystem::CheckEnemyDead(GameData& gameData){
    std::vector<DenseSlotMap<Enemy>::Key> toRemove;
    for (auto& enemy : gameData.enemies) {
        if (enemy.m_currentHealth <= 0.0f)
            toRemove.push_back(gameData.enemies.KeyOf(&enemy));
    }
    for (auto& key : toRemove) {
        gameData.gold += gameData.enemies.Get(key)->m_reward;
        RemoveEnemy(key, gameData);
    }
}

void WorldSystem::TickAttacks(float dt, GameData& gameData){
    for (auto& attack : gameData.attacks) {
        if (!attack.m_resolved) {
            attack.m_delay -= dt;
            if (attack.m_delay <= 0.0f) {
                for (auto& key : attack.m_targetKeys) {
                    Enemy* enemy = gameData.enemies.Get(key);
                    if (!enemy) continue;
                    float armor = 0.0f;
                    for (auto& mod : enemy->m_modules)
                        if (auto* a = dynamic_cast<ArmorModule*>(mod.get()))
                            armor += a->m_amount;
                    enemy->m_currentHealth -= std::max(0.0f, attack.m_damage - armor);
                    for (auto& effect : attack.m_effects)
                        enemy->AddEffect(effect);
                }
                attack.m_resolved = true;
            }
        }
        attack.m_duration -= dt;
    }
    std::erase_if(gameData.attacks, [](const Attack& a){ return a.m_duration <= 0.0f; });
}

void WorldSystem::CheckGameOver(bool& gameOver, GameData& gameData){
    // Core live reaches zero
    if(gameData.lives <= 0){
        gameOver = true;
    }
}
