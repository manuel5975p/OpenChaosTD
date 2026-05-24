#pragma once

#include <game.hpp>

class EnemySystem{
public:

    void FollowPath(float dt, GameData& gameData);
    void TickEnemies(float dt, GameData& gameData, ParticleSystem& particles);

private:
    
};