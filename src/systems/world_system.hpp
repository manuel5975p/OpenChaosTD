#pragma once

#include <game.hpp>
#include <world/tower.hpp>
#include <world/enemy.hpp>

class WorldSystem{
public:
    void PlaceTower(int x, int y, Tower& towerTemplate, GameData& gameData);
    void RemoveTower(int x, int y, GameData& gameData);
    void SpawnEnemy(const int& nest, const Enemy& templateEnemy,GameData& gameData);
    void RemoveEnemy();

    void CheckEnemyReachedCore(GameData& gameData);
    void GenerateMap(Map& map, int x, int y);
    void CheckGameOver(bool& gameOver, GameData& gameData);
    std::vector<DenseSlotMap<Enemy>::Key> SelectTargets(Tower& tower, DenseSlotMap<Enemy>& enemies, int max_targets = 1);

private:
    bool ValidateTowerPlacement(int x, int y, GameData& gameData);
    std::vector<Enemy*> GetEnemiesInTowerRange(Tower& tower, DenseSlotMap<Enemy>& enemies);
    bool CompareTarget(const Enemy& a, const Enemy& b, TargetingMode mode);
};