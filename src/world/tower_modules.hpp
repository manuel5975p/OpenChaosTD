#pragma once

class TowerModule{
    public:
    virtual ~TowerModule() = default;
};

class FlatDamageModule : public TowerModule{
    public:
    float m_damage;
    FlatDamageModule(float damage) : m_damage(damage) {}
};

class SlowModule : public TowerModule{
    public:
    float m_factor, m_duration;
    SlowModule(float factor, float duration) : m_factor(factor), m_duration(duration) {}
};