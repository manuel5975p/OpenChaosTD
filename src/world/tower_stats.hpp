#pragma once

#include <string>

enum class TargetingMode {
    First,
    Last,
    MostHealth,
    LowestHealth,
    Fastest,
    Slowest,
    MostArmor,
    MostResistance,
    MostShield
};

inline constexpr int kTargetingModeCount = static_cast<int>(TargetingMode::MostShield) + 1; // MostShield stays last

inline TargetingMode NextTargetingMode(TargetingMode m) {
    return static_cast<TargetingMode>((static_cast<int>(m) + 1) % kTargetingModeCount);
}

inline const char* TargetingModeName(TargetingMode m) {
    switch (m) {
        case TargetingMode::First:          return "First";
        case TargetingMode::Last:           return "Last";
        case TargetingMode::MostHealth:     return "Most HP";
        case TargetingMode::LowestHealth:   return "Least HP";
        case TargetingMode::Fastest:        return "Fastest";
        case TargetingMode::Slowest:        return "Slowest";
        case TargetingMode::MostArmor:      return "Most Armor";
        case TargetingMode::MostResistance: return "Most Resist";
        case TargetingMode::MostShield:     return "Most Shield";
    }
    return "First";
}

struct TowerStats {
    float damage = 0.0f;
    float shotsPerMinute = 0.0f; // cooldown between shots = 60 / shotsPerMinute
    float range = 0.0f;
    int targetCount = 0;
    TargetingMode targetingMode = TargetingMode::First;
    float armorPierce = 0.0f;
    float critChance = 0.0f;     // 0 = no crit
    float critMultiplier = 1.0f;
};

// Apply an upgrade delta to a named stat field (additive, or multiplicative when mul=true).
inline void ApplyStat(TowerStats& s, const std::string& key, float v, bool mul) {
    auto apply = [&](float& field) { field = mul ? field * v : field + v; };
    if      (key == "damage")         apply(s.damage);
    else if (key == "shotsPerMinute") apply(s.shotsPerMinute);
    else if (key == "range")          apply(s.range);
    else if (key == "armorPierce")    apply(s.armorPierce);
    else if (key == "critChance")     apply(s.critChance);
    else if (key == "critMultiplier") apply(s.critMultiplier);
    else if (key == "targetCount") {
        float t = static_cast<float>(s.targetCount);
        apply(t);
        s.targetCount = static_cast<int>(t + 0.5f);
    }
}
