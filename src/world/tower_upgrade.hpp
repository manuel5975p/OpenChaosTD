#pragma once

#include <string>
#include <vector>
#include <utility>
#include <cstdio>
#include <unordered_map>
#include <toml++/toml.hpp>
#include <world/tower_modules.hpp>

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

// One additive delta line: "+10% Crit" for fraction keys, "+20% Slow" for whole-percent keys,
// "+2 Range" for flat values.
inline std::string FormatAddDelta(const std::string& key, float v) {
    char buf[48];
    if (IsFractionKey(key))     snprintf(buf, sizeof(buf), "+%g%% %s", v * 100.0f, StatLabel(key));
    else if (IsPercentKey(key)) snprintf(buf, sizeof(buf), "+%g%% %s", v, StatLabel(key));
    else                        snprintf(buf, sizeof(buf), "+%g %s", v, StatLabel(key));
    return buf;
}

// One multiplicative delta line: "x1.5 Rate".
inline std::string FormatMulDelta(const std::string& key, float v) {
    char buf[48];
    snprintf(buf, sizeof(buf), "x%g %s", v, StatLabel(key));
    return buf;
}

// One purchasable upgrade level for a tower: scalar deltas broadcast to its modules
// (additive + multiplicative) plus any new effect modules to append.
struct TowerUpgrade {
    int m_cost = 0;
    std::vector<std::pair<std::string, float>> m_adds; // stat key -> additive delta
    std::vector<std::pair<std::string, float>> m_muls; // stat key -> multiplicative factor
    std::vector<toml::table> m_addModules;             // new effect modules built via TowerFactory

    // Append this level's deltas as display lines, mirroring TowerModule::Describe.
    void Describe(std::vector<DescLine>& out) const;
};

inline void TowerUpgrade::Describe(std::vector<DescLine>& out) const {
    for (const auto& [key, v] : m_adds)
        out.push_back({FormatAddDelta(key, v), RAYWHITE});
    for (const auto& [key, v] : m_muls)
        out.push_back({FormatMulDelta(key, v), RAYWHITE});
    for (const auto& mod : m_addModules) {
        std::string type = mod["type"].value_or(std::string{});
        if (!type.empty()) out.push_back({"Adds " + type, RAYWHITE});
    }
}
