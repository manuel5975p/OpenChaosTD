#pragma once

#include <vector>
#include <memory>
#include <raylib.h>
#include <string>

#include <engine/lib/dense_slotmap.hpp>
#include <world/enemy.hpp>
#include <world/tower_modules.hpp>
#include <world/tower_visual.hpp>
#include <world/tower_upgrade.hpp>

class Tower {
public:
    std::string m_name;
    std::string m_description;
    Vector2 m_position;

    int m_cost = 0;

    TowerVisual m_visual; // set by factory, never modified at runtime

    float m_cooldown         = 0.0f;
    float m_attackFlashRatio = 0.0f;
    std::vector<std::unique_ptr<TowerModule>> m_modules;

    int m_level = 0;
    const std::vector<TowerUpgrade>* m_upgrades = nullptr; // stable pointer into the factory template

    void AddModule(std::unique_ptr<TowerModule> mod) {
        // Cache the AttackModule (if any) so systems can cheaply tell shooters from walls and
        // read combat stats without scanning the module list every frame.
        if (auto* attack = dynamic_cast<AttackModule*>(mod.get()))
            m_attack = attack;
        m_modules.push_back(std::move(mod));
    }

    // Non-null iff this tower can attack; null marks a wall/passive tower.
    AttackModule* GetAttack() const { return m_attack; }

private:
    AttackModule* m_attack = nullptr; // points into m_modules; set in AddModule
};
