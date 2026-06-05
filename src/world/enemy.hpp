#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <memory>
#include <world/effect.hpp>
#include <world/enemy_modules.hpp>
#include <world/enemy_stats.hpp>
#include <world/enemy_upgrade.hpp>
#include <engine/features/particle_system.hpp>

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
    const EmitterDesc* m_deathDescPtr = nullptr; // stable pointer into EmitterPresets; burst emitted on death

    EnemyStats m_base;   // set from JSON, never modified at runtime
    EnemyStats m_stats;  // recomputed each tick from base + ContributeStats modules

    float m_progress = 0.0f;
    int   m_spawnedNest   = 0;
    int   m_waypointIndex = -1;

    std::vector<Effect> m_effects;
    std::vector<std::unique_ptr<EnemyModule>> m_modules;

    int m_level = 0;
    const std::vector<EnemyUpgrade>* m_upgrades = nullptr; // stable pointer into the factory template

    // Patch a stat by key: base stats route to the Enemy's own fields, everything else is
    // forwarded to the modules (shield, armor, regenRate, splitCount, ...). Mirrors the
    // generic PatchStats pipeline; ApplyDelta is shared from tower_modules.hpp.
    void PatchStats(const std::string& key, float v, bool mul) {
        if (key == "maxHealth") {
            ApplyDelta(m_maxHealth, v, mul);
            m_currentHealth = m_maxHealth; // a scaled/elite enemy spawns at full HP
        } else if (key == "speed") {
            ApplyDelta(m_base.m_speed, v, mul); // live m_stats is recomputed from base next tick
        } else if (key == "reward") {
            float r = static_cast<float>(m_reward);
            ApplyDelta(r, v, mul);
            m_reward = static_cast<int>(r + 0.5f);
        } else {
            for (auto& mod : m_modules)
                mod->PatchStats(key, v, mul);
        }
    }

    void AddModule(std::unique_ptr<EnemyModule> mod) {
        // Cache the ShieldModule (if any) so the damage and targeting hot paths can read the
        // shield without scanning the module list or dynamic-casting every frame.
        if (auto* shield = dynamic_cast<ShieldModule*>(mod.get()))
            m_shield = shield;
        m_modules.push_back(std::move(mod));
    }

    // Non-null iff this enemy carries a shield; points into m_modules, set in AddModule.
    ShieldModule* GetShield() const { return m_shield; }

    void AddEffect(Effect effect) {
        for (const auto& mod : m_modules)
            if (mod->ShouldBlock(effect.m_type)) return;

        // Effects don't stack; reapply only refreshes (timer + value) when equal or stronger
        for (auto& existing : m_effects) {
            if (existing.m_type != effect.m_type) continue;
            if (effect.m_value >= existing.m_value) existing = effect;
            return;
        }
        m_effects.push_back(std::move(effect));
    }

    Effect* FindEffect(EffectType type) {
        for (auto& e : m_effects)
            if (e.m_type == type) return &e;
        return nullptr;
    }

    void RemoveEffect(EffectType type) {
        std::erase_if(m_effects, [type](const Effect& e){ return e.m_type == type; });
    }

private:
    ShieldModule* m_shield = nullptr; // points into m_modules; set in AddModule
};
