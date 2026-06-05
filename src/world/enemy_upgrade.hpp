#pragma once

#include <string>
#include <vector>
#include <utility>
#include <cstdio>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <world/tower_modules.hpp>

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

// One additive delta line: "+50 Health", "+5 Armor".
inline std::string FormatEnemyAddDelta(const std::string& key, float v) {
    char buf[48];
    snprintf(buf, sizeof(buf), "+%g %s", v, EnemyStatLabel(key));
    return buf;
}

// One multiplicative delta line: "x2 Health".
inline std::string FormatEnemyMulDelta(const std::string& key, float v) {
    char buf[48];
    snprintf(buf, sizeof(buf), "x%g %s", v, EnemyStatLabel(key));
    return buf;
}

// One upgrade level for an enemy: scalar deltas routed through Enemy::PatchStats
// (additive + multiplicative) plus any new modules to append. Mirrors TowerUpgrade.
// m_cost is kept for structural symmetry with TowerUpgrade (future "threat budget"); unused for now.
struct EnemyUpgrade {
    int m_cost = 0;
    std::vector<std::pair<std::string, float>> m_adds; // stat key -> additive delta
    std::vector<std::pair<std::string, float>> m_muls; // stat key -> multiplicative factor
    std::vector<nlohmann::json> m_addModules;          // new modules built via EnemyFactory

    // Append this level's deltas as display lines, mirroring TowerUpgrade::Describe.
    void Describe(std::vector<DescLine>& out) const;
};

inline void EnemyUpgrade::Describe(std::vector<DescLine>& out) const {
    for (const auto& [key, v] : m_adds)
        out.push_back({FormatEnemyAddDelta(key, v), RAYWHITE});
    for (const auto& [key, v] : m_muls)
        out.push_back({FormatEnemyMulDelta(key, v), RAYWHITE});
    for (const auto& mod : m_addModules) {
        std::string type = mod.value("type", "");
        if (!type.empty()) out.push_back({"Adds " + type, RAYWHITE});
    }
}
