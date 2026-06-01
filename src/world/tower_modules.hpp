#pragma once

#include <string>
#include <raylib.h>
#include <engine/features/particle_system.hpp>
#include <world/tower_stats.hpp>

struct AttackPayload;

class TowerModule {
public:
    virtual ~TowerModule() = default;
    virtual void Contribute(AttackPayload&) const {}
    virtual void ContributeTower(TowerStats&) const {}
    virtual void Describe(std::string&, Color&) const {}
    // Called by the upgrade system to patch module-owned parameters (e.g. slowFactor, burnDamage).
    virtual void PatchStat(const std::string& /*key*/, float /*v*/, bool /*mul*/) {}
};

class SlowModule : public TowerModule {
public:
    float m_factor, m_duration;
    EmitterDesc m_particleDesc;
    float m_emitRate = 8.0f;
    SlowModule(float factor, float duration, EmitterDesc particleDesc);
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
    void PatchStat(const std::string& key, float v, bool mul) override;
};

class BurnModule : public TowerModule {
public:
    float m_value, m_duration;
    EmitterDesc m_particleDesc;
    float m_emitRate = 18.0f;
    BurnModule(float value, float duration, EmitterDesc particleDesc);
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
    void PatchStat(const std::string& key, float v, bool mul) override;
};
