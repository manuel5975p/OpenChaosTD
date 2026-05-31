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
    bool m_wasCrit = false;
    std::vector<Effect> m_effects;

    std::vector<EmitterDesc> m_impactDescs;
    std::vector<EmitterDesc> m_critImpactDescs;  // emitted in addition to m_impactDescs on crit

    EmitterDesc m_trailDesc;

    float m_delay = 0.0f;
    bool m_resolved = false;
    float m_ttl = 0.0f;  // expires after attackDuration so it outlives the crit roll
};
