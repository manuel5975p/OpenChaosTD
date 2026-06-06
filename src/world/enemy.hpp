#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include <memory>
#include <world/effect.hpp>
#include <world/enemy_modules.hpp>
#include <world/enemy_upgrade.hpp>
#include <world/enemy_visual.hpp>
#include <engine/features/particle_system.hpp>

class Enemy {
public:
    std::string m_name;
    std::string m_description;
    Vector2 m_position = {0.0f, 0.0f};

    EnemyVisual m_visual; // set by factory, never modified at runtime

    // Runtime hit points. Max/speed/reward/livesOnReach live in the BaseStatsModule (see GetBaseStats);
    // current health is the live counterpart that damage and regen mutate, like the tower's m_cooldown.
    float m_currentHealth = 0.0f;

    float m_progress = 0.0f;
    int   m_spawnedNest   = 0;
    int   m_waypointIndex = -1;

    std::vector<Effect> m_effects;
    std::vector<std::unique_ptr<EnemyModule>> m_modules;

    int m_level = 0;
    const std::vector<EnemyUpgrade>* m_upgrades = nullptr; // stable pointer into the factory template

    // Patch a stat by key: broadcast to every module (the BaseStatsModule handles the core stats,
    // the rest handle shield, armor, regenRate, splitCount, ...). Mirrors the generic PatchStats
    // pipeline; ApplyDelta is shared from tower_modules.hpp.
    void PatchStats(const std::string& key, float v, bool mul) {
        for (auto& mod : m_modules)
            mod->PatchStats(key, v, mul);
        // A scaled/elite enemy spawns at full HP, so re-sync current health after maxHealth changes.
        if (key == "maxHealth" && m_baseStats)
            m_currentHealth = m_baseStats->m_maxHealth;
    }

    // Deep-copy this enemy (modules included). Used to spawn from a pre-upgraded prototype and to
    // hand fully-upgraded copies to the HUD. Enemy is move-only (its modules are unique_ptr), so the
    // implicit copy is deleted; cloning rebuilds each module via its virtual Clone() and re-adds it
    // through AddModule, which re-caches m_baseStats/m_shield into the new module vector (order
    // preserved). m_upgrades is a stable pointer into the factory template, so a shallow copy is
    // correct (mirrors EnemyFactory::Create); m_level is copied as-is.
    Enemy Clone() const {
        Enemy copy;
        copy.m_name          = m_name;
        copy.m_description    = m_description;
        copy.m_position       = m_position;
        copy.m_visual         = m_visual;
        copy.m_currentHealth  = m_currentHealth;
        copy.m_progress       = m_progress;
        copy.m_spawnedNest    = m_spawnedNest;
        copy.m_waypointIndex  = m_waypointIndex;
        copy.m_effects        = m_effects;
        copy.m_level          = m_level;
        copy.m_upgrades       = m_upgrades;
        for (const auto& mod : m_modules)
            copy.AddModule(mod->Clone());
        return copy;
    }

    void AddModule(std::unique_ptr<EnemyModule> mod) {
        // Cache the BaseStatsModule and ShieldModule (if present) so the damage and targeting hot
        // paths can read core/shield stats without scanning the module list every frame.
        if (auto* base = dynamic_cast<BaseStatsModule*>(mod.get()))
            m_baseStats = base;
        if (auto* shield = dynamic_cast<ShieldModule*>(mod.get()))
            m_shield = shield;
        m_modules.push_back(std::move(mod));
    }

    // Always non-null for a factory-built enemy; points into m_modules, set in AddModule. Owns the
    // core stats (maxHealth/speed/reward/livesOnReach) and the live speed/armor mirror.
    BaseStatsModule* GetBaseStats() const { return m_baseStats; }

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
    BaseStatsModule* m_baseStats = nullptr; // points into m_modules; set in AddModule
    ShieldModule* m_shield = nullptr;       // points into m_modules; set in AddModule
};
