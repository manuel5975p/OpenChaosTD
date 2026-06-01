#include <systems/world_system.hpp>
#include <world/tile.hpp>
#include <factory/enemy_factory.hpp>
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

        gameData.map.BuildPathMesh();
    }
}

bool WorldSystem::ValidateTowerPlacement(int x, int y, GameData& gameData){
    Tile& tile = gameData.map.Get(x, y);

    // Return if tile not buildable
    if(!tile.m_buildable)
        return false;

    // Check if paths are still valid after tower placement
    tile.m_walkable = false;
    gameData.map.BuildPathMesh();
    if(!gameData.map.ValidatePathMesh()){
        tile.m_walkable = true;
        gameData.map.BuildPathMesh();
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

void WorldSystem::CheckEnemyDead(GameData& gameData, EnemyFactory& enemyFactory, ParticleSystem& particles){
    std::vector<DenseSlotMap<Enemy>::Key> toRemove;
    for (auto& enemy : gameData.enemies) {
        if (enemy.m_currentHealth <= 0.0f)
            toRemove.push_back(gameData.enemies.KeyOf(&enemy));
    }
    for (auto& key : toRemove) {
        Enemy* enemy = gameData.enemies.Get(key);
        gameData.gold += enemy->m_reward;

        // Copy parent path state and collect spawn requests before mutating the slotmap
        Vector2 pos          = enemy->m_position;
        int     nest         = enemy->m_spawnedNest;
        int     waypoint     = enemy->m_waypointIndex;
        float   progress     = enemy->m_progress;

        std::vector<SpawnRequest> requests;
        for (const auto& mod : enemy->m_modules) {
            auto req = mod->OnDeath();
            if (req && enemyFactory.Has(req->type))
                requests.push_back(*req);
        }

        // Death burst — pointer into EmitterPresets, set at enemy creation time
        if (enemy->m_deathDescPtr)
            particles.Emit(pos, *enemy->m_deathDescPtr);

        RemoveEnemy(key, gameData);

        // Spawn children after parent is removed
        for (const auto& req : requests) {
            for (int i = 0; i < req.count; i++) {
                Enemy child = enemyFactory.Create(req.type);
                child.m_position     = pos;
                child.m_spawnedNest  = nest;
                child.m_waypointIndex = waypoint;
                child.m_progress     = progress;
                gameData.enemies.Insert(std::move(child));
            }
        }
    }
}


void WorldSystem::CheckGameOver(bool& gameOver, GameData& gameData){
    // Core live reaches zero
    if(gameData.lives <= 0){
        gameOver = true;
    }
}
