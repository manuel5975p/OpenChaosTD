#pragma once

#include <string>
#include <optional>
#include <raylib.h>
#include <world/effect.hpp>
#include <world/enemy_stats.hpp>

class Enemy;

struct SpawnRequest {
    std::string type;
    int count;
};

class EnemyModule {
public:
    virtual ~EnemyModule() = default;
    virtual void Tick(float, Enemy&) const {}
    virtual void ContributeStats(EnemyStats&) const {}
    virtual float InterceptDamage(float incoming) const { return incoming; }
    virtual std::optional<SpawnRequest> OnDeath() const { return std::nullopt; }
    virtual bool ShouldBlock(EffectType) const { return false; }
    virtual float GetShield() const { return 0.0f; }
    virtual void Describe(std::string&, Color&) const {}
};

class RegenerationModule : public EnemyModule {
public:
    float m_rate;
    explicit RegenerationModule(float rate) : m_rate(rate) {}
    void Tick(float dt, Enemy& enemy) const override;
    void Describe(std::string& text, Color& color) const override;
};

class ArmorModule : public EnemyModule {
public:
    float m_amount;
    explicit ArmorModule(float amount) : m_amount(amount) {}
    void ContributeStats(EnemyStats& stats) const override;
    void Describe(std::string& text, Color& color) const override;
};

// Enemy ignores any incoming effect of the given type
class ImmuneModule : public EnemyModule {
public:
    EffectType m_effect;
    explicit ImmuneModule(EffectType effect) : m_effect(effect) {}
    bool ShouldBlock(EffectType type) const override;
    void Describe(std::string& text, Color& color) const override;
};

// Absorbs incoming damage until depleted
class ShieldModule : public EnemyModule {
public:
    float m_maxShield;
    mutable float m_currentShield;
    explicit ShieldModule(float shield) : m_maxShield(shield), m_currentShield(shield) {}
    float GetShield() const override { return m_currentShield; }
    float InterceptDamage(float incoming) const override;
    void Describe(std::string& text, Color& color) const override;
};

// On death, spawns m_count children of type m_childType at the death position
class SplitModule : public EnemyModule {
public:
    std::string m_childType;
    int m_count;
    SplitModule(std::string childType, int count)
        : m_childType(std::move(childType)), m_count(count) {}
    std::optional<SpawnRequest> OnDeath() const override;
    void Describe(std::string& text, Color& color) const override;
};
