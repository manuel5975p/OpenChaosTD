#pragma once

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
