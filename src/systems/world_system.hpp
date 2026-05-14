#pragma once

#include <game.hpp>
#include <world/tower.hpp>
#include <world/enemy.hpp>

class WorldSystem{
public:
    bool PlaceTower(int x, int y, Tower& tower, GameData& gameData);
    void RemoveTower(int x, int y, GameData& gameData);
    void SpawnEnemy(int nest, Enemy&& enemy, GameData& gameData);
    void RemoveEnemy(DenseSlotMap<Enemy>::Key key, GameData& gameData);

    void CheckEnemyReachedCore(GameData& gameData);
    void CheckEnemyDead(GameData& gameData);
    void TickAttacks(float dt, GameData& gameData);
    void GenerateMap(Map& map, int x, int y);
    void CheckGameOver(bool& gameOver, GameData& gameData);

private:
    bool ValidateTowerPlacement(int x, int y, GameData& gameData);
};