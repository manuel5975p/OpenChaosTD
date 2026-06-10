#pragma once

#include <vector>
#include <memory>
#include <raylib.h>
#include <string>

#include <world/tower_modules.hpp>
#include <world/tower_presentation.hpp>
#include <world/tower_upgrade.hpp>

// Unlike Enemy, Tower has no Clone(): a tower is built exactly once by TowerFactory::Create and
// then moved into the world's slotmap, never spawned from a prototype. It is move-only (its modules
// are unique_ptr) and intentionally stays that way — there is no spawn-from-prototype path that would
// need a deep copy, so no Clone()/sizeof tripwire is needed (contrast Enemy::Clone()).
class Tower {
public:
    std::string m_name;
    std::string m_description;
    Vector2 m_position;

    int m_cost = 0;

    TowerPresentation m_presentation; // set by factory, never modified at runtime
    TowerAnimation m_animation;       // per-frame visual state (attack flash); driven by TowerSystem

    float m_cooldown = 0.0f;
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

    // Broadcast a stat delta to every module by key; each module patches only the keys it owns
    // (AttackModule -> core combat stats, effect modules -> their own params). Mirrors
    // Enemy::PatchStats; shared by the upgrade pipeline and tile-based terrain buffs.
    void PatchStats(const std::string& key, float v, bool mul) {
        for (auto& mod : m_modules)
            mod->PatchStats(key, v, mul);
    }

private:
    AttackModule* m_attack = nullptr; // points into m_modules; set in AddModule
};
