#pragma once

#include <string>
#include <unordered_map>
#include <world/upgrade_shared.hpp>

// Readable name for an enemy upgrade key (base stats and module parameters); falls back to the raw key.
inline const char* EnemyStatLabel(const std::string& key) {
    static const std::unordered_map<std::string, const char*> labels = {
        {"maxHealth", "Health"}, {"speed", "Speed"},      {"reward", "Reward"},
        {"armor", "Armor"},      {"shield", "Shield"},    {"regenRate", "Regen"},
        {"splitCount", "Split"},
    };
    auto it = labels.find(key);
    return it != labels.end() ? it->second : key.c_str();
}

// Formatting policy for enemy upgrade lines (no fraction/percent keys).
inline const StatFormat& EnemyStatFormat() {
    static const StatFormat fmt{EnemyStatLabel, NoSpecialKey, NoSpecialKey};
    return fmt;
}

// One upgrade step for an enemy: the shared scalar deltas / added modules. Mirrors TowerUpgrade,
// minus the player-facing cost (enemy upgrades are applied by wave tier, not purchased).
struct EnemyUpgrade : UpgradeDef {
    void Describe(std::vector<DescLine>& out) const { UpgradeDef::Describe(out, EnemyStatFormat()); }
};
