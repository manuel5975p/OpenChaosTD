#pragma once

#include <vector>
#include <world/effect.hpp>
#include <world/enemy.hpp>
#include <engine/features/particle_system.hpp>
#include <engine/lib/dense_slotmap.hpp>

struct AttackPayload {
    std::vector<DenseSlotMap<Enemy>::Key> m_targetKeys;

    float m_damage = 0.0f;
    float m_armorPierce = 0.0f;
    float m_critChance = 0.0f;
    float m_critMultiplier = 1.0f;
    std::vector<Effect> m_effects;

    std::vector<EmitterDesc> m_impactDescs;
    std::vector<EmitterDesc> m_critImpactDescs;  // emitted in addition to m_impactDescs on crit

    bool m_resolved = false; // damage applied once; erased the same tick it resolves
};
