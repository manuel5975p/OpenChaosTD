#pragma once

#include <string>
#include <unordered_map>
#include <world/upgrade_shared.hpp>

// Readable name for an upgrade key (combat stats and module parameters); falls back to the raw key.
inline const char* StatLabel(const std::string& key) {
    static const std::unordered_map<std::string, const char*> labels = {
        {"damage", "Damage"},          {"shotsPerMinute", "Rate"},       {"range", "Range"},
        {"targetCount", "Targets"},    {"armorPierce", "Pierce"},        {"critChance", "Crit"},
        {"critMultiplier", "Crit Mult"}, {"slowPercent", "Slow"},        {"slowDuration", "Slow Time"},
        {"burnDamage", "Burn"},        {"burnDuration", "Burn Time"},    {"shredAmount", "Shred"},
        {"shredDuration", "Shred Time"}, {"weaknessAmount", "Weakness"}, {"weaknessDuration", "Weak Time"},
        {"stunDuration", "Stun Time"}, {"bonusPerStack", "Ramp"},        {"maxStacks", "Max Stacks"},
        {"idleTime", "Idle"},
    };
    auto it = labels.find(key);
    return it != labels.end() ? it->second : key.c_str();
}

// Keys stored as a fraction in [0..1]: render the value scaled to a percentage (0.1 -> "10%").
inline bool IsFractionKey(const std::string& key) {
    return key == "critChance";
}

// Keys already stored as a whole percentage: render with "%" but without scaling (20 -> "20%").
inline bool IsPercentKey(const std::string& key) {
    return key == "slowPercent";
}

// Formatting policy for tower upgrade lines.
inline const StatFormat& TowerStatFormat() {
    static const StatFormat fmt{StatLabel, IsFractionKey, IsPercentKey};
    return fmt;
}

// One purchasable upgrade level for a tower: the shared scalar deltas / added modules plus a cost.
struct TowerUpgrade : UpgradeDef {
    int m_cost = 0;

    void Describe(std::vector<DescLine>& out) const { UpgradeDef::Describe(out, TowerStatFormat()); }
};
