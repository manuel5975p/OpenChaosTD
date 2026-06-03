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
    // Called by the upgrade system to patch module-owned parameters (e.g. slowPercent, burnDamage).
    virtual void PatchStat(const std::string& /*key*/, float /*v*/, bool /*mul*/) {}
    // Stateful hooks: Tick runs every frame (idle decay), OnFire runs the frame the tower fires.
    virtual void Tick(float /*dt*/) {}
    virtual void OnFire() {}
};

class SlowModule : public TowerModule {
public:
    float m_slowPercent, m_duration; // m_slowPercent: slow strength as a percent (90 = 90% slower)
    EmitterDesc m_particleDesc;
    float m_emitRate = 8.0f;
    SlowModule(float slowPercent, float duration, EmitterDesc particleDesc);
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
    void PatchStat(const std::string& key, float v, bool mul) override;
};

class BurnModule : public TowerModule {
public:
    float m_damage, m_duration;
    EmitterDesc m_particleDesc;
    float m_emitRate = 18.0f;
    BurnModule(float value, float duration, EmitterDesc particleDesc);
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
    void PatchStat(const std::string& key, float v, bool mul) override;
};

class ArmorShredModule : public TowerModule {
public:
    float m_amount, m_duration; // m_amount: flat armor removed while active
    EmitterDesc m_particleDesc;
    float m_emitRate = 12.0f;
    ArmorShredModule(float amount, float duration, EmitterDesc particleDesc);
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
    void PatchStat(const std::string& key, float v, bool mul) override;
};

class WeaknessModule : public TowerModule {
public:
    float m_amount, m_duration; // m_amount: flat bonus damage the next hit deals
    EmitterDesc m_particleDesc;
    float m_emitRate = 10.0f;
    WeaknessModule(float amount, float duration, EmitterDesc particleDesc);
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
    void PatchStat(const std::string& key, float v, bool mul) override;
};

class StunModule : public TowerModule {
public:
    float m_duration;
    EmitterDesc m_particleDesc;
    float m_emitRate = 14.0f;
    StunModule(float duration, EmitterDesc particleDesc);
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
    void PatchStat(const std::string& key, float v, bool mul) override;
};

// Supplies the attack's critical-hit chance and multiplier (the roll happens in damage resolution)
class CritModule : public TowerModule {
public:
    float m_critChance, m_critMultiplier; // m_critChance in [0..1]; m_critMultiplier applied on a crit
    CritModule(float critChance, float critMultiplier);
    void Contribute(AttackPayload& attack) const override;
    void Describe(std::string& text, Color& color) const override;
    void PatchStat(const std::string& key, float v, bool mul) override;
};

// Self-buff: each shot adds a stack (capped) that raises fire rate; stacks clear after idle time
class SlowStartModule : public TowerModule {
public:
    float m_bonusPerStack, m_idleTime;
    int m_maxStacks;
    int m_stacks = 0;
    float m_idleTimer = 0.0f;
    SlowStartModule(float bonusPerStack, int maxStacks, float idleTime);
    void ContributeTower(TowerStats& stats) const override;
    void Tick(float dt) override;
    void OnFire() override;
    void Describe(std::string& text, Color& color) const override;
    void PatchStat(const std::string& key, float v, bool mul) override;
};
