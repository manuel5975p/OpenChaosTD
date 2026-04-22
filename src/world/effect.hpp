#pragma once

enum class EffectType {
    Burn
};

struct Effect {
public:
    EffectType m_type;
    float m_duration;
    float m_value;

    Effect(EffectType type, float duration, float value)
        : m_type(type), m_duration(duration), m_value(value) {}
};