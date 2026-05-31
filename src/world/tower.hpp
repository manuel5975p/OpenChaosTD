#pragma once

#include <vector>
#include <memory>
#include <raylib.h>
#include <string>

#include <engine/lib/dense_slotmap.hpp>
#include <world/enemy.hpp>
#include <world/tower_modules.hpp>
#include <world/tower_stats.hpp>
#include <world/tower_vfx.hpp>

enum class TowerRole { Shooter, Wall };

class Tower {
public:
    std::string m_name;
    std::string m_description;
    std::string m_texture;
    Vector2 m_position;
    TowerRole m_role = TowerRole::Shooter;

    int m_cost = 0;

    TowerStats m_base;   // set from JSON, never modified at runtime
    TowerStats m_stats;  // recomputed each tick from base + ContributeTower modules

    TowerVfxDesc m_vfx; // set by factory, never modified at runtime

    float m_cooldown         = 0.0f;
    float m_attackFlashRatio = 0.0f;
    std::vector<std::unique_ptr<TowerModule>> m_modules;

    void AddModule(std::unique_ptr<TowerModule> mod) {
        m_modules.push_back(std::move(mod));
    }
};
