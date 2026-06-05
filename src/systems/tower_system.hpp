#pragma once

#include <world/game_data.hpp>
#include <engine/features/particle_system.hpp>

class TowerSystem{
public:
    void Update(float dt, GameData& gameData, ParticleSystem& particles);
    void TickAttacks(float dt, GameData& gameData, ParticleSystem& particles);
    std::vector<DenseSlotMap<Enemy>::Key> FindTargets(const Tower& tower, DenseSlotMap<Enemy>& enemies, int max_targets);

private:
    void RecomputeStats(Tower& tower, float dt);
    void DecayAttackFlash(Tower& tower, float dt);
    void Fire(Tower& tower, const std::vector<DenseSlotMap<Enemy>::Key>& targetKeys, GameData& gameData, ParticleSystem& particles);
    static AttackVisual BuildVisual(const Tower& tower, std::vector<Vector2> targetPositions);
    void BuildPayload(const Tower& tower, AttackPayload& payload);
    bool CompareTarget(const Enemy& a, const Enemy& b, TargetingMode mode);
    std::vector<Enemy*> FindEnemiesInRange(const Tower& tower, DenseSlotMap<Enemy>& enemies);
};