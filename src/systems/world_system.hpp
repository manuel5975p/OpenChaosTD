#pragma once

#include <game.hpp>
#include <world/tower.hpp>

class WorldSystem{
public:
    void PlaceTower(int x, int y, Tower& towerTemplate, GameData& gameData);
    void RemoveTower(int x, int y, GameData& gameData);

    void SpawnEnemy(const int& nest, const Enemy& templateEnemy,GameData& gameData);
    void RemoveEnemy();
    
    void UpdateEnemyPosition(float& dt,GameData& gameData);
    void CheckEnemyReachedCore(GameData& gameData);

    void GenerateMap(Map& map, int x, int y);
    void CheckGameOver(bool& gameOver, GameData& gameData);


private:
    bool ValidateTowerPlacement(int x, int y, GameData& gameData);
};