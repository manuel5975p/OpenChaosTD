#pragma once

#include <string>
#include <raylib.h>
#include <world/tower_stats.hpp>

struct Attack;

class TowerModule {
public:
    virtual ~TowerModule() = default;
    virtual void Contribute(Attack&) const {}
    virtual void ContributeTower(TowerStats&) const {}
    virtual void Describe(std::string&, Color&) const {}
};

class FlatDamageModule : public TowerModule {
public:
    float m_damage;
    FlatDamageModule(float damage) : m_damage(damage) {}
    void Contribute(Attack& attack) const override;
    void Describe(std::string& text, Color& color) const override;
};

class SlowModule : public TowerModule {
public:
    float m_factor, m_duration;
    SlowModule(float factor, float duration) : m_factor(factor), m_duration(duration) {}
    void Contribute(Attack& attack) const override;
    void Describe(std::string& text, Color& color) const override;
};

class BurnModule : public TowerModule {
public:
    float m_value, m_duration;
    BurnModule(float value, float duration) : m_value(value), m_duration(duration) {}
    void Contribute(Attack& attack) const override;
    void Describe(std::string& text, Color& color) const override;
};

class ArmorPiercingModule : public TowerModule {
public:
    float m_pierce;
    explicit ArmorPiercingModule(float pierce) : m_pierce(pierce) {}
    void Contribute(Attack& attack) const override;
    void Describe(std::string& text, Color& color) const override;
};

class CritModule : public TowerModule {
public:
    float m_chance, m_multiplier;
    CritModule(float chance, float multiplier) : m_chance(chance), m_multiplier(multiplier) {}
    void Contribute(Attack& attack) const override;
    void Describe(std::string& text, Color& color) const override;
};
