#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <raylib.h>
#include <world/effect.hpp>
#include <world/tower_modules.hpp> // DescLine + ApplyDelta shared module utilities

class Enemy;
class BaseStatsModule;

struct SpawnRequest {
    std::string m_type;
    int m_count;
    float m_spacing = 0.0f; // world-unit gap between consecutive children so they don't perfectly overlap
};

class EnemyModule {
public:
    virtual ~EnemyModule() = default;
    // Deep-copy this module (virtual-constructor idiom) so an Enemy prototype can be cloned.
    virtual std::unique_ptr<EnemyModule> Clone() const = 0;
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

    std::unique_ptr<EnemyModule> Clone() const override { return std::make_unique<BaseStatsModule>(*this); }
    void ResetLive(); // copy base -> live, called at the start of each recompute
    void PatchStats(const std::string& key, float v, bool mul) override;
    // Core stat rows (Health/Speed), reading the live values.
    void DescribeStats(std::vector<DescLine>& out) const override;
};

class RegenerationModule : public EnemyModule {
public:
    float m_rate;
    explicit RegenerationModule(float rate) : m_rate(rate) {}
    std::unique_ptr<EnemyModule> Clone() const override { return std::make_unique<RegenerationModule>(*this); }
    void Tick(float dt, Enemy& enemy) override;
    void DescribeStats(std::vector<DescLine>& out) const override;
    void PatchStats(const std::string& key, float v, bool mul) override;
};

class ArmorModule : public EnemyModule {
public:
    float m_amount;
    explicit ArmorModule(float amount) : m_amount(amount) {}
    std::unique_ptr<EnemyModule> Clone() const override { return std::make_unique<ArmorModule>(*this); }
    void ContributeStats(BaseStatsModule& base) const override;
    void DescribeStats(std::vector<DescLine>& out) const override;
    void PatchStats(const std::string& key, float v, bool mul) override;
};

// Enemy ignores any incoming effect of the given type
class ImmuneModule : public EnemyModule {
public:
    EffectType m_effect;
    explicit ImmuneModule(EffectType effect) : m_effect(effect) {}
    std::unique_ptr<EnemyModule> Clone() const override { return std::make_unique<ImmuneModule>(*this); }
    bool ShouldBlock(EffectType type) const override;
    void DescribeStats(std::vector<DescLine>& out) const override;
};

// Absorbs incoming damage until depleted
class ShieldModule : public EnemyModule {
public:
    float m_maxShield;
    float m_currentShield;
    explicit ShieldModule(float shield) : m_maxShield(shield), m_currentShield(shield) {}
    std::unique_ptr<EnemyModule> Clone() const override { return std::make_unique<ShieldModule>(*this); }
    float GetShield() const override { return m_currentShield; }
    float InterceptDamage(float incoming) override;
    void DescribeStats(std::vector<DescLine>& out) const override;
    void PatchStats(const std::string& key, float v, bool mul) override;
};

// On death, spawns m_count children of type m_childType, staggered along the path by m_spacing so the
// children spread out instead of stacking on the exact death position.
class SplitModule : public EnemyModule {
public:
    std::string m_childType;
    int m_count;
    float m_spacing; // world-unit gap between consecutive children (0 = stack on the death position)
    SplitModule(std::string childType, int count, float spacing)
        : m_childType(std::move(childType)), m_count(count), m_spacing(spacing) {}
    std::unique_ptr<EnemyModule> Clone() const override { return std::make_unique<SplitModule>(*this); }
    std::optional<SpawnRequest> OnDeath() const override;
    void DescribeStats(std::vector<DescLine>& out) const override;
    void PatchStats(const std::string& key, float v, bool mul) override;
};
