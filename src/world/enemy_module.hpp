#pragma once

#include <string>
#include <world/effect.hpp>

class EnemyModule {
public:
    virtual ~EnemyModule() = default;
};

class RegenerationModule : public EnemyModule {
public:
    float m_rate;
    explicit RegenerationModule(float rate) : m_rate(rate) {}
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
};

// Enemy ignores any incoming effect of the given type
class ImmuneModule : public EnemyModule {
public:
    EffectType m_effect;
    explicit ImmuneModule(EffectType effect) : m_effect(effect) {}
};

// On death, spawns m_count children of type m_childType at the death position
class SplitModule : public EnemyModule {
public:
    std::string m_childType;
    int m_count;
    SplitModule(std::string childType, int count)
        : m_childType(std::move(childType)), m_count(count) {}
};
