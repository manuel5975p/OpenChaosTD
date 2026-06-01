#pragma once

#include <world/game_data.hpp>
#include <engine/features/particle_system.hpp>

class TowerSystem{
public:
    void Update(float dt, GameData& gameData, ParticleSystem& particles);
    void TickPayloads(GameData& gameData, ParticleSystem& particles);
    void TickVfx(float dt, GameData& gameData);
    std::vector<DenseSlotMap<Enemy>::Key> FindTargets(Tower& tower, DenseSlotMap<Enemy>& enemies, int max_targets);

private:
    void BuildPayload(const Tower& tower, AttackPayload& payload);
    bool CompareTarget(const Enemy& a, const Enemy& b, TargetingMode mode);
    std::vector<Enemy*> FindEnemiesInRange(Tower& tower, DenseSlotMap<Enemy>& enemies);
};