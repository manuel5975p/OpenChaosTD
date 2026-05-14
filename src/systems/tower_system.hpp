#pragma once

#include <game.hpp>

class TowerSystem{
public:
    void update(float dt, GameData& gameData);
    void TickAttacks(float dt, GameData& gameData);
    std::vector<DenseSlotMap<Enemy>::Key> FindTargets(Tower& tower, DenseSlotMap<Enemy>& enemies, int max_targets);

private:
    void BuildAttackPayload(const Tower& tower, Attack& attack);
    bool CompareTarget(const Enemy& a, const Enemy& b, TargetingMode mode);
    std::vector<Enemy*> FindEnemiesInRange(Tower& tower, DenseSlotMap<Enemy>& enemies);
};