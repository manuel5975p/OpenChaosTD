#pragma once

#include <string>
#include <raylib.h>

struct Attack;

class TowerModule {
public:
    virtual ~TowerModule() = default;
    virtual void Contribute(Attack&) const {}
    virtual std::string Describe() const { return ""; }
    virtual Color DescribeColor() const { return RAYWHITE; }
};

class FlatDamageModule : public TowerModule {
public:
    float m_damage;
    FlatDamageModule(float damage) : m_damage(damage) {}
    void Contribute(Attack& attack) const override;
    std::string Describe() const override;
};

class SlowModule : public TowerModule {
public:
    float m_factor, m_duration;
    SlowModule(float factor, float duration) : m_factor(factor), m_duration(duration) {}
    void Contribute(Attack& attack) const override;
    std::string Describe() const override;
    Color DescribeColor() const override;
};
