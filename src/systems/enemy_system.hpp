#pragma once

#include <world/game_data.hpp>
#include <core/particle_system.hpp>

class EnemySystem{
public:

    void FollowPath(float dt, GameData& gameData);
    void TickEnemies(float dt, GameData& gameData, ParticleSystem& particles);

private:
    
};