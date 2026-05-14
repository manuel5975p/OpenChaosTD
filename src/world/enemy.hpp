#pragma once

#include <raylib.h>
#include <string>
#include <vector>

#include <memory>
#include <world/effect.hpp>
#include <world/enemy_module.hpp>

class Enemy{
public:
    std::string m_name;
    std::string m_texture;
    Vector2 m_position      = {0.0f, 0.0f};
    
    float m_health          = 0.0f;
    float m_currentHealth   = 0.0f;
    float m_speed           = 0.0f;
    float m_currentSpeed    = 0.0f;
    int   m_reward          = 0;

    float m_progress        = 0.0f;
    int   m_spawnedNest     = 0;
    int   m_waypointIndex   = -1;

    std::vector<Effect> m_effects;
    std::vector<std::unique_ptr<EnemyModule>> m_modules;

    void AddModule(std::unique_ptr<EnemyModule> mod) {
        m_modules.push_back(std::move(mod));
    }

    void AddEffect(Effect effect) {
        for (auto& existing : m_effects) {
            if (existing.m_type != effect.m_type) continue;
            // Replace only if the incoming effect is stronger
            switch (effect.m_type) {
                case EffectType::Burn: if (effect.m_value > existing.m_value) existing = effect; break;
                case EffectType::Slow: if (effect.m_value < existing.m_value) existing = effect; break;
            }
            return;
        }
        m_effects.push_back(std::move(effect));
    }
};