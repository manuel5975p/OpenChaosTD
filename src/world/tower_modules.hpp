#pragma once

#include <string>
#include <raylib.h>
#include <core/particle_system.hpp>
#include <world/tower_stats.hpp>

struct AttackPayload;

class TowerModule {
public:
    virtual ~TowerModule() = default;
    virtual void Contribute(AttackPayload&) const {}
    virtual void ContributeTower(TowerStats&) const {}
    virtual void Describe(std::string&, Color&) const {}
};

class FlatDamageModule : public TowerModule {
public:
    float m_damage;
    FlatDamageModule(float damage) : m_damage(damage) {}
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
};

class SlowModule : public TowerModule {
public:
    float m_factor, m_duration;
    EmitterDesc m_particleDesc;
    float m_emitRate = 8.0f;
    SlowModule(float factor, float duration, EmitterDesc particleDesc);
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
};

class BurnModule : public TowerModule {
public:
    float m_value, m_duration;
    EmitterDesc m_particleDesc;
    float m_emitRate = 18.0f;
    BurnModule(float value, float duration, EmitterDesc particleDesc);
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
};

class ArmorPiercingModule : public TowerModule {
public:
    float m_pierce;
    explicit ArmorPiercingModule(float pierce) : m_pierce(pierce) {}
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
};

class CritModule : public TowerModule {
public:
    float m_chance, m_multiplier;
    CritModule(float chance, float multiplier) : m_chance(chance), m_multiplier(multiplier) {}
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
};
