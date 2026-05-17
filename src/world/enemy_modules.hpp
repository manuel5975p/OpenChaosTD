#pragma once

#include <string>
#include <optional>
#include <world/effect.hpp>

class Enemy;

struct SpawnRequest {
    std::string type;
    int count;
};

class EnemyModule {
public:
    virtual ~EnemyModule() = default;
    virtual void Tick(float, Enemy&) const {}
    virtual std::optional<SpawnRequest> OnDeath() const { return std::nullopt; }
    virtual bool ShouldBlock(EffectType) const { return false; }
};

class RegenerationModule : public EnemyModule {
public:
    float m_rate;
    explicit RegenerationModule(float rate) : m_rate(rate) {}
    void Tick(float dt, Enemy& enemy) const override;
};

class ArmorModule : public EnemyModule {
public:
    float m_amount;
    explicit ArmorModule(float amount) : m_amount(amount) {}
};

class ResistanceModule : public EnemyModule {
public:
    float m_factor;
    explicit ResistanceModule(float factor) : m_factor(factor) {}
    void Tick(float dt, Enemy& enemy) const override;
};

// Enemy ignores any incoming effect of the given type
class ImmuneModule : public EnemyModule {
public:
    EffectType m_effect;
    explicit ImmuneModule(EffectType effect) : m_effect(effect) {}
    bool ShouldBlock(EffectType type) const override;
};

// On death, spawns m_count children of type m_childType at the death position
class SplitModule : public EnemyModule {
public:
    std::string m_childType;
    int m_count;
    SplitModule(std::string childType, int count)
        : m_childType(std::move(childType)), m_count(count) {}
    std::optional<SpawnRequest> OnDeath() const override;
};
