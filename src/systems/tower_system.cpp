#include <systems/tower_system.hpp>

#include <world/tower.hpp>

void TowerSystem::UpdateTowers(float& dt, GameData& gameData){
    for (Tower& tower : gameData.towers) {
        tower.m_cooldown -= dt;

        // Attack
        if(tower.m_cooldown <= 0){
            std::cout << "Attack" << std::endl;
            tower.m_cooldown = 1.0f / tower.m_fireRate;
        }
    }
}