#pragma once

#include <string>
#include <vector>
#include <optional>
#include <raylib.h>
#include <world/effect.hpp>
#include <world/tower_modules.hpp> // DescLine + ApplyDelta shared module utilities

class Enemy;
class BaseStatsModule;

struct SpawnRequest {
    std::string type;
    int count;
};

class EnemyModule {
public:
    virtual ~EnemyModule() = default;
    // Stateful per-frame hook (e.g. RegenerationModule); non-const so modules mutate cleanly.
    virtual void Tick(float, Enemy&) {}
    // Augment the enemy's live combat stats each tick (e.g. ArmorModule feeds m_liveArmor).
    // The mirror of TowerModule::ContributeTower(AttackModule&).
    virtual void ContributeStats(BaseStatsModule&) const {}
    // Stateful damage hook (e.g. ShieldModule depletes its pool); non-const for the same reason.
    virtual float InterceptDamage(float incoming) { return incoming; }
    virtual std::optional<SpawnRequest> OnDeath() const { return std::nullopt; }
    virtual bool ShouldBlock(EffectType) const { return false; }
    virtual float GetShield() const { return 0.0f; }
    // Append this module's display lines to the enemy info panel (zero or more rows).
    virtual void DescribeStats(std::vector<DescLine>&) const {}
    // Patch module-owned parameters for dynamic scaling (mirrors TowerModule::PatchStats).
    virtual void PatchStats(const std::string& /*key*/, float /*v*/, bool /*mul*/) {}
};

// The "core" enemy module: owns an enemy's base stats (formerly the Enemy scalars + EnemyStats).
// The direct analogue of the tower's AttackModule. Base fields are the configured/upgraded values;
// the m_liveX mirror is reset from base each tick and augmented by other modules' ContributeStats
// (e.g. ArmorModule feeds m_liveArmor). Every enemy carries exactly one, built by EnemyFactory.
class BaseStatsModule : public EnemyModule {
public:
    // Base config — mutated only by upgrades via PatchStats.
    float m_maxHealth = 0.0f;
    float m_speed = 0.0f;
    int   m_reward = 0;
    int   m_livesOnReach = 1;

    // Live combat stats for the current tick. Reset from base by ResetLive(); armor has no innate
    // base (it is contributed entirely by ArmorModule). Current health is persistent runtime state
    // and lives on Enemy, so it is intentionally not reset here.
    float m_liveSpeed = 0.0f;
    float m_liveArmor = 0.0f;

    void ResetLive(); // copy base -> live, called at the start of each recompute
    void PatchStats(const std::string& key, float v, bool mul) override;
    // Core stat rows (Health/Speed), reading the live values.
    void DescribeStats(std::vector<DescLine>& out) const override;
};

class RegenerationModule : public EnemyModule {
public:
    float m_rate;
    explicit RegenerationModule(float rate) : m_rate(rate) {}
    void Tick(float dt, Enemy& enemy) override;
    void DescribeStats(std::vector<DescLine>& out) const override;
    void PatchStats(const std::string& key, float v, bool mul) override;
};

class ArmorModule : public EnemyModule {
public:
    float m_amount;
    explicit ArmorModule(float amount) : m_amount(amount) {}
    void ContributeStats(BaseStatsModule& base) const override;
    void DescribeStats(std::vector<DescLine>& out) const override;
    void PatchStats(const std::string& key, float v, bool mul) override;
};

// Enemy ignores any incoming effect of the given type
class ImmuneModule : public EnemyModule {
public:
    EffectType m_effect;
    explicit ImmuneModule(EffectType effect) : m_effect(effect) {}
    bool ShouldBlock(EffectType type) const override;
    void DescribeStats(std::vector<DescLine>& out) const override;
};

// Absorbs incoming damage until depleted
class ShieldModule : public EnemyModule {
public:
    float m_maxShield;
    float m_currentShield;
    explicit ShieldModule(float shield) : m_maxShield(shield), m_currentShield(shield) {}
    float GetShield() const override { return m_currentShield; }
    float InterceptDamage(float incoming) override;
    void DescribeStats(std::vector<DescLine>& out) const override;
    void PatchStats(const std::string& key, float v, bool mul) override;
};

// On death, spawns m_count children of type m_childType at the death position
class SplitModule : public EnemyModule {
public:
    std::string m_childType;
    int m_count;
    SplitModule(std::string childType, int count)
        : m_childType(std::move(childType)), m_count(count) {}
    std::optional<SpawnRequest> OnDeath() const override;
    void DescribeStats(std::vector<DescLine>& out) const override;
    void PatchStats(const std::string& key, float v, bool mul) override;
};
