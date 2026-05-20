#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <memory>
#include <world/effect.hpp>
#include <world/enemy_modules.hpp>
#include <world/enemy_stats.hpp>
#include <world/particle.hpp>

class Enemy {
public:
    std::string m_name;
    std::string m_description;
    std::string m_texture;
    Vector2 m_position = {0.0f, 0.0f};

    float m_maxHealth     = 0.0f;
    float m_currentHealth = 0.0f;
    int   m_reward        = 0;
    int   m_livesOnReach  = 1;
    EmitterDesc m_deathDesc; // burst emitted when this enemy dies

    EnemyStats m_base;   // set from JSON, never modified at runtime
    EnemyStats m_stats;  // recomputed each tick from base + ContributeStats modules

    float m_progress = 0.0f;
    int   m_spawnedNest   = 0;
    int   m_waypointIndex = -1;

    std::vector<Effect> m_effects;
    std::vector<std::unique_ptr<EnemyModule>> m_modules;

    void AddModule(std::unique_ptr<EnemyModule> mod) {
        m_modules.push_back(std::move(mod));
    }

    void AddEffect(Effect effect) {
        for (const auto& mod : m_modules)
            if (mod->ShouldBlock(effect.m_type)) return;

        for (auto& existing : m_effects) {
            if (existing.m_type != effect.m_type) continue;
            switch (effect.m_type) {
                case EffectType::Burn: if (effect.m_value > existing.m_value) existing = effect; break;
                case EffectType::Slow: if (effect.m_value <= existing.m_value) existing = effect; break;
            }
            return;
        }
        m_effects.push_back(std::move(effect));
    }
};
