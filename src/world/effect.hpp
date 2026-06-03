#pragma once

#include <engine/features/particle_system.hpp>

enum class EffectType {
    Burn,
    Slow,
    ArmorShred, // flat armor reduction while active (min 0)
    Stun,       // can't move; cleared when the enemy is hit
    Weakness    // next hit deals flat bonus damage, then is consumed
};

struct Effect {
public:
    EffectType m_type;
    float m_duration;
    float m_value; // per-type magnitude & "strength": Burn dps, Slow %, Shred amount, Weakness bonus dmg, Stun = its duration

    // Particle emission — set by the module that created this effect
    EmitterDesc m_particleDesc;
    float m_emitRate = 0.0f;        // particles/sec; 0 = no visual
    float m_emitAccumulator = 0.0f;

    Effect(EffectType type, float duration, float value)
        : m_type(type), m_duration(duration), m_value(value) {}
};
