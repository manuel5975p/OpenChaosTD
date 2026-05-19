#pragma once

#include <world/particle.hpp>

enum class EffectType {
    Burn,
    Slow
};

struct Effect {
public:
    EffectType m_type;
    float m_duration;
    float m_value;

    // Particle emission — set by the module that created this effect
    EmitterDesc m_particleDesc;
    float m_emitRate = 0.0f;        // particles/sec; 0 = no visual
    float m_emitAccumulator = 0.0f;

    Effect(EffectType type, float duration, float value)
        : m_type(type), m_duration(duration), m_value(value) {}
};
