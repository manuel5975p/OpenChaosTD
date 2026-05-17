#pragma once

#include <string>
#include <vector>
#include <raylib.h>

struct Attack;

struct StatRow {
    std::string text;
    Color color;
};

class TowerModule {
public:
    virtual ~TowerModule() = default;
    virtual void Contribute(Attack&) const {}
    virtual void Describe(std::vector<StatRow>&) const {}
};

class FlatDamageModule : public TowerModule {
public:
    float m_damage;
    FlatDamageModule(float damage) : m_damage(damage) {}
    void Contribute(Attack& attack) const override;
    void Describe(std::vector<StatRow>& rows) const override;
};

class SlowModule : public TowerModule {
public:
    float m_factor, m_duration;
    SlowModule(float factor, float duration) : m_factor(factor), m_duration(duration) {}
    void Contribute(Attack& attack) const override;
    void Describe(std::vector<StatRow>& rows) const override;
};
